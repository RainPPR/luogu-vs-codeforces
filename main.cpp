#include <bits/stdc++.h>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

using namespace std;

// -----------------------------------------------------------------------------

int run(string op) {
	return system(op.c_str());
}

unordered_set<string> get_luogu(uint64_t uid) {
	string s_uid = to_string(uid);
	string filename = "./data/" + s_uid + ".json";

	run("curl https://www.luogu.com.cn/user/" + s_uid + "?_contentOnly=1 -o " + filename);

	ifstream f(filename);
	unordered_set<string> ac_luogu_cf;

	auto problems = json::parse(f)["currentData"]["passedProblems"];
	for (auto problem : problems) {
		if (problem["type"] != "CF")
			continue;
		ac_luogu_cf.insert(string(problem["pid"]).substr(2));
	}

	return ac_luogu_cf;
}

unordered_set<string> get_codeforces(string name) {
	string filename = "./data/" + name + ".json";

	run("curl https://ojhunt.com/api/crawlers/codeforces/" + name + " -o " + filename);

	ifstream f(filename);
	unordered_set<string> ac_cf;

	auto problems = json::parse(f)["data"]["solvedList"];
	for (auto problem : problems)
		ac_cf.insert(problem);

	return ac_cf;
}

string get_title(string pid) {
	string filename = "./data/CF" + pid + ".json";

	run("curl https://www.luogu.com.cn/problem/CF" + pid + "?_contentOnly=1 -o " + filename);

	ifstream f(filename);

	auto file = json::parse(f);
	if (file["code"] != 200)
		return "";

	return file["currentData"]["problem"]["title"];
}

// -----------------------------------------------------------------------------

signed main() {
	run("mkdir data");

	auto luogu = get_luogu(371511);
	auto codeforces = get_codeforces("RainPPR");

	ofstream fout("table.csv");
	fout << "pid,title,origin,luogu,vjudge" << endl;

	for (auto pid : codeforces) {
		if (luogu.count(pid) != 0)
			continue;

		string title = get_title(pid);
		string luogu_url = "https://www.luogu.com.cn/problem/CF" + pid;
		string vjudge_url = "https://vjudge.net/problem/CodeForces-" + pid;

		if (title.empty())
			luogu_url = "";
		fout << "CF" + pid << ",\"" << title << "\",CodeForces," + luogu_url + "," + vjudge_url << endl;
	}

	fout.close();
	return 0;
}

/*
-std=c++17 -O2 -pipe -s -static
*/

