#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>

using namespace std;
struct bill {
    int nominal;
    int count;
};
struct testcase {
    vector<bill> wallet;
    int amount;
    string strategy;
};
vector<testcase> parsejson(const string& filename) {
    vector<testcase> cases;
    ifstream file(filename);
    if (!file.is_open()) {
        return cases;
    }
    string content = "";
    string line;
    while (getline(file, line)) {
        content += line + "\n";
    }
    file.close();
    int len = content.length();
    int pos = 0;
    while (pos < len) {
        pos = content.find("{", pos);
        if (pos == -1) {
            break;
        }
        testcase tc;
        int w_pos = content.find("\"wallet\"", pos);
        if (w_pos != -1) {
            int start = content.find("[", w_pos) + 1;
            int end = start;
            int balance = 1;
            while (end < len && balance > 0) {
                if (content[end] == '[') balance++;
                if (content[end] == ']') balance--;
                end++;
            }
            string w_str = content.substr(start, end - start - 1);
            int sub = 0;
            while ((sub = w_str.find("[", sub)) != -1) {
                int comma = w_str.find(",", sub);
                int close = w_str.find("]", sub);
                bill b;
                b.nominal = stoi(w_str.substr(sub + 1, comma - sub - 1));
                b.count = stoi(w_str.substr(comma + 1, close - comma - 1));
                tc.wallet.push_back(b);
                sub = close + 1;
            }
        }
        int a_pos = content.find("\"amount\"", pos);
        if (a_pos != -1) {
            int colon = content.find(":", a_pos);
            tc.amount = stoi(content.substr(colon + 1));
        }
        int s_pos = content.find("\"strategy\"", pos);
        if (s_pos != -1) {
            int q1 = content.find("\"", s_pos + 10);
            int q2 = content.find("\"", q1 + 1);
            tc.strategy = content.substr(q1 + 1, q2 - q1 - 1);
        }
        cases.push_back(tc);
        pos = content.find("}", pos) + 1;
    }
    return cases;
}
vector<int> res_counts;
bool success = false;
long long max_weight = -1;
long long min_weight = -1;
int best_diff = 2e9;

void evaluate(const vector<int>& current, const vector<bill>& wallet, const string& strat) {
    int n = wallet.size();
    if (strat == "MAX") {
        long long weight = 0;
        long long mult = 1;
        for (int i = 0; i < n; i++) {
            weight += (long long)current[i] * mult;
            mult *= 15000;
        }
        if (weight > max_weight) {
            max_weight = weight;
            res_counts = current;
            success = true;
        }
    }
    else if (strat == "MIN") {
        long long weight = 0;
        long long mult = 1;
        for (int i = n - 1; i >= 0; i--) {
            weight += (long long)current[i] * mult;
            mult *= 15000;
        }
        if (weight > min_weight) {
            min_weight = weight;
            res_counts = current;
            success = true;
        }
    }
    else if (strat == "UNIFORM") {
        int mn = 2e9, mx = -1;
        for (int i = 0; i < n; i++) {
            if (current[i] < mn) {
                mn = current[i];
            }
            if (current[i] > mx) {
                mx = current[i];
            }
        }
        int diff = mx - mn;
        if (diff < best_diff) {
            best_diff = diff;
            res_counts = current;
            success = true;
        }
    }
}
void solve(int idx, int rem, vector<int>& current, const vector<bill>& wallet, const string& strat) {
    if (rem == 0) {
        evaluate(current, wallet, strat);
        return;
    }
    if (idx < 0 || rem < 0) {
        return;
    }
        int nom = wallet[idx].nominal;
        int max_take = min(wallet[idx].count, rem / nom);
    if (strat == "MAX") {
        for (int i = max_take; i >= 0; i--) {
            current[idx] = i;
            solve(idx - 1, rem - i * nom, current, wallet, strat);
            if (success && idx == wallet.size() - 1) {
                break;
            }
        }
    }
    else if (strat == "MIN") {
        for (int i = 0; i <= max_take; i++) {
            current[idx] = i;
            solve(idx - 1, rem - i * nom, current, wallet, strat);
            if (success && idx == wallet.size() - 1) {
                break;
            }
        }
    }
    else {
        for (int i = 0; i <= max_take; i++) {
            current[idx] = i;
            solve(idx - 1, rem - i * nom, current, wallet, strat);
        }
    }
    current[idx] = 0;
}
int main() {
    vector<testcase> cases = parsejson("input.json");
    ofstream fout("output.json");
    fout << "[\n";
    int cases_count = cases.size();
    for (int t = 0; t < cases_count; t++) {
        testcase& tc = cases[t];
        sort(tc.wallet.begin(), tc.wallet.end(), [](const bill& a, const bill& b) {
            return a.nominal < b.nominal;
            });
        int n = tc.wallet.size();
        res_counts.assign(n, 0);
        success = false;
        max_weight = -1;
        min_weight = -1;
        best_diff = 2e9;
        vector<int> current(n, 0);
        solve(n - 1, tc.amount, current, tc.wallet, tc.strategy);
        fout << "  {\n    \"dispense\": [";
        if (success) {
            bool first = true;
            for (int i = 0; i < n; i++) {
                if (res_counts[i] > 0) {
                    if (!first) {
                        fout << ", ";
                    }
                    fout << "[" << tc.wallet[i].nominal << ", " << res_counts[i] << "]";
                    first = false;
                }
            }
        }
        fout << "]\n  }";
        if (t < cases_count - 1) {
            fout << ",\n";
		}
        else {
            fout << "\n";
        }
    }
    fout << "]\n";
    fout.close();
    return 0;
}