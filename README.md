# [luogu-vs-codeforces](https://github.com/RainPPR/luogu-vs-codeforces)

# 声明：本脚本仅供个人学习使用！

### 介绍

众所周知（截止到 2024-09）洛谷的 CodeForces RMJ 很不稳定，很多人选择在 CodeForces 原站 / Vjudge 提交。

这个脚本可以自动爬取 CodeForces 原站的通过记录，自动与洛谷的通过记录匹配并将已在 CodeForces 通过但是洛谷没有通过的题目记录在根目录 `table.csv` 里面。

### 软件架构

使用 C++ (C++17) 编写，使用 <https://github.com/nlohmann/json> 作为解析 json 的库。

使用洛谷官方 api 读取洛谷提交记录，形如 `https://www.luogu.com.cn/user/{uid}?_contentOnly=1`。

使用 ojhunt 的 api 读取 CodeForces 提交记录，形如 `https://ojhunt.com/api/crawlers/codeforces/{username}`。

后期可能加入 ojhunt 的 vjudge 接口（暂时不考虑加入，因为作者本人习惯在 vjudge 绑定 CodeForces 账号）。

另外，曾经考虑使用 vjudge 官方 api 但是在 Github Actions 上面抓包会出问题（只会 curl 嘤嘤嘤）。

### 安装教程

本地运行：

1.  Clone [这个仓库](https://github.com/RainPPR/luogu-vs-codeforces)。
2.  修改 [`main.cpp`](https://github.com/RainPPR/luogu-vs-codeforces/blob/main/main.cpp) 中主函数 `main()` 中的洛谷 uid 和 CodeForces 用户名。
3.  将 nlohmann 的 [json.hpp](https://ghp.ci/https://github.com/nlohmann/json/releases/download/v3.11.3/json.hpp) 复制到同一文件夹，编译。
4.  确保您的电脑已安装 curl 并在 path 中或在同一文件夹，运行后 `table.csv` 在同一文件夹下。

在 Github 中使用 Actions：

1.  Fork [这个仓库](https://github.com/RainPPR/luogu-vs-codeforces)。
2.  修改 [`main.cpp`](https://github.com/RainPPR/luogu-vs-codeforces/blob/main/main.cpp) 中主函数 `main()` 中的洛谷 uid 和 CodeForces 用户名。
4.  启用 Actions 并手动运行 Build 工作流，运行后 `table.csv` 在根目录。

### 开发教程

以下是对代码 [`main.cpp`](https://github.com/RainPPR/luogu-vs-codeforces/blob/main/main.cpp) 的说明：

```cpp
int run(string op) {
	return system(op.c_str());
}
```

用于运行 bash 或 cmd 命令。

```cpp
unordered_set<string> get_luogu(uint64_t uid) {
	string s_uid = to_string(uid);
	string filename = "./data/" + s_uid + ".json";

	run("curl https://www.luogu.com.cn/user/" + s_uid + "?_contentOnly=1 -o " + filename);

	ifstream f(filename);
	unordered_set<string> ac_luogu_cf;

	auto problems = json::parse(f)["currentData"]["passedProblems"];
	for (auto problem : problems) {
		if (problem["type"] != "CF")
			continue;	// Vjudge
		ac_luogu_cf.insert(string(problem["pid"]).substr(2));
	}

	return ac_luogu_cf;
}
```

用于获取洛谷上的通过记录，返回一个 `unordered_set<string>` 类型，包含不带 `CF` 前缀的题号（pid）。

```cpp
unordered_set<string> get_codeforces(string name) {
	string filename = "./data/" + name + ".json";

	run("curl https://ojhunt.com/api/crawlers/codeforces/" + name + " -o " + filename);

	ifstream f(filename);
	unordered_set<string> ac_cf;

	auto problems = json::parse(f)["data"]["solvedList"];
	for (string problem : problems) {
		if (problem.size() >= 7)
			continue;	// CodeForces Gym
		ac_cf.insert(problem);
	}

	return ac_cf;
}
```

用于获取 CodeForces 上的通过记录，注意 ojhunt 返回的结果包含 Gym 的题目，我们粗略的使用长度来判断是 problemset 还是 Gym。

此处，注意到目前 CodeForces 题目 problemset 最长形如 CFxxxxE1，去掉前缀为 6 位；Gym 题目大多为 7 位。

```cpp
string get_title(string pid) {
	string filename = "./data/CF" + pid + ".json";

	static int tot = 0;
	cout << "GET #" << ++tot << endl;

	run("curl https://www.luogu.com.cn/problem/CF" + pid + "?_contentOnly=1 -s -o " + filename);

	ifstream f(filename);

	auto file = json::parse(f);
	if (file["code"] != 200)
		return "";

	return file["currentData"]["problem"]["title"];
}
```

通过洛谷的接口获取题目标题，用于标识题目的同时检测是否洛谷没有题（Div.1 和 Div.2 共题时洛谷仅包含一个，另一个返回 404）。

```cpp
run("mkdir data");

auto luogu = get_luogu(371511);
auto codeforces = get_codeforces("RainPPR");
```

创建文件夹和获取信息，注意到如果没有 data 文件夹可能会 curl 错误。

```cpp
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
```

输出到文件，不考虑 curl 的复杂度，此段代码时间复杂度为 O(N)（？）
