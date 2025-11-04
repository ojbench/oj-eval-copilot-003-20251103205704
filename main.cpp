#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <sstream>

using namespace std;

struct Submission {
    char problem;
    string status;
    int time;
    int submissionId;  // Global insertion order
};

struct ProblemStatus {
    bool solved = false;
    int solveTime = 0;
    int wrongAttempts = 0;
    int frozenSubmissions = 0;
    int frozenWrongAttempts = 0;  // Wrong attempts during frozen period
    vector<Submission> submissions;
};

struct Team {
    string name;
    map<char, ProblemStatus> problems;
    int solvedCount = 0;
    int penaltyTime = 0;
    int ranking = 0;
    
    void updateStats(int problemCount) {
        solvedCount = 0;
        penaltyTime = 0;
        for (char p = 'A'; p < 'A' + problemCount; p++) {
            if (problems[p].solved) {
                solvedCount++;
                penaltyTime += problems[p].solveTime + 20 * problems[p].wrongAttempts;
            }
        }
    }
};

class ICPCSystem {
private:
    map<string, Team> teams;
    vector<string> teamNames;
    bool started = false;
    bool frozen = false;
    int durationTime = 0;
    int problemCount = 0;
    int freezeTime = -1;
    int nextSubmissionId = 0;  // Global submission counter
    
    bool compareTeams(const string& a, const string& b) {
        Team& ta = teams[a];
        Team& tb = teams[b];
        
        if (ta.solvedCount != tb.solvedCount) {
            return ta.solvedCount > tb.solvedCount;
        }
        if (ta.penaltyTime != tb.penaltyTime) {
            return ta.penaltyTime < tb.penaltyTime;
        }
        
        // Compare solve times
        vector<int> timesA, timesB;
        for (char p = 'A'; p < 'A' + problemCount; p++) {
            if (ta.problems[p].solved) {
                timesA.push_back(ta.problems[p].solveTime);
            }
            if (tb.problems[p].solved) {
                timesB.push_back(tb.problems[p].solveTime);
            }
        }
        sort(timesA.rbegin(), timesA.rend());
        sort(timesB.rbegin(), timesB.rend());
        
        int minSize = min(timesA.size(), timesB.size());
        for (int i = 0; i < minSize; i++) {
            if (timesA[i] != timesB[i]) {
                return timesA[i] < timesB[i];
            }
        }
        
        return a < b;
    }
    
    void updateRankings() {
        for (auto& t : teams) {
            t.second.updateStats(problemCount);
        }
        
        sort(teamNames.begin(), teamNames.end(), [this](const string& a, const string& b) {
            return compareTeams(a, b);
        });
        
        for (int i = 0; i < teamNames.size(); i++) {
            teams[teamNames[i]].ranking = i + 1;
        }
    }
    
    string getProblemDisplay(const ProblemStatus& ps, bool isFrozen) {
        if (isFrozen) {
            return (ps.wrongAttempts > 0 ? "-" : "") + to_string(ps.wrongAttempts) + 
                   "/" + to_string(ps.frozenSubmissions);
        } else if (ps.solved) {
            if (ps.wrongAttempts == 0) {
                return "+";
            }
            return "+" + to_string(ps.wrongAttempts);
        } else {
            if (ps.wrongAttempts == 0) {
                return ".";
            }
            return "-" + to_string(ps.wrongAttempts);
        }
    }
    
    void printScoreboard() {
        for (const auto& name : teamNames) {
            const Team& t = teams[name];
            cout << name << " " << t.ranking << " " << t.solvedCount << " " << t.penaltyTime;
            for (char p = 'A'; p < 'A' + problemCount; p++) {
                cout << " ";
                const ProblemStatus& ps = t.problems.at(p);
                bool isFrozen = frozen && !ps.solved && ps.frozenSubmissions > 0;
                cout << getProblemDisplay(ps, isFrozen);
            }
            cout << "\n";
        }
    }
    
public:
    void addTeam(const string& name) {
        if (started) {
            cout << "[Error]Add failed: competition has started.\n";
            return;
        }
        if (teams.count(name)) {
            cout << "[Error]Add failed: duplicated team name.\n";
            return;
        }
        teams[name] = Team{name};
        teamNames.push_back(name);
        cout << "[Info]Add successfully.\n";
    }
    
    void startCompetition(int duration, int problems) {
        if (started) {
            cout << "[Error]Start failed: competition has started.\n";
            return;
        }
        started = true;
        durationTime = duration;
        problemCount = problems;
        
        for (auto& t : teams) {
            for (char p = 'A'; p < 'A' + problemCount; p++) {
                t.second.problems[p] = ProblemStatus();
            }
        }
        
        sort(teamNames.begin(), teamNames.end());
        for (int i = 0; i < teamNames.size(); i++) {
            teams[teamNames[i]].ranking = i + 1;
        }
        
        cout << "[Info]Competition starts.\n";
    }
    
    void submit(char problem, const string& teamName, const string& status, int time) {
        Team& team = teams[teamName];
        ProblemStatus& ps = team.problems[problem];
        
        ps.submissions.push_back({problem, status, time, nextSubmissionId++});
        
        if (frozen) {
            if (!ps.solved) {
                ps.frozenSubmissions++;
            }
        } else {
            if (!ps.solved) {
                if (status == "Accepted") {
                    ps.solved = true;
                    ps.solveTime = time;
                } else {
                    ps.wrongAttempts++;
                }
            }
        }
    }
    
    void flush() {
        updateRankings();
        cout << "[Info]Flush scoreboard.\n";
    }
    
    void freeze() {
        if (frozen) {
            cout << "[Error]Freeze failed: scoreboard has been frozen.\n";
            return;
        }
        frozen = true;
        cout << "[Info]Freeze scoreboard.\n";
    }
    
    void scroll() {
        if (!frozen) {
            cout << "[Error]Scroll failed: scoreboard has not been frozen.\n";
            return;
        }
        
        cout << "[Info]Scroll scoreboard.\n";
        
        // Flush first
        updateRankings();
        printScoreboard();
        
        // Process scrolling
        while (true) {
            bool hasAnyFrozen = false;
            string lowestTeam = "";
            int lowestRanking = -1;
            
            for (const auto& name : teamNames) {
                const Team& t = teams[name];
                bool hasFrozen = false;
                for (char p = 'A'; p < 'A' + problemCount; p++) {
                    const ProblemStatus& ps = t.problems.at(p);
                    if (!ps.solved && ps.frozenSubmissions > 0) {
                        hasFrozen = true;
                        break;
                    }
                }
                if (hasFrozen) {
                    hasAnyFrozen = true;
                    if (t.ranking > lowestRanking) {
                        lowestRanking = t.ranking;
                        lowestTeam = name;
                    }
                }
            }
            
            if (!hasAnyFrozen) break;
            
            // Find smallest problem number to unfreeze
            char smallestProblem = 'Z' + 1;
            Team& team = teams[lowestTeam];
            for (char p = 'A'; p < 'A' + problemCount; p++) {
                ProblemStatus& ps = team.problems[p];
                if (!ps.solved && ps.frozenSubmissions > 0) {
                    smallestProblem = p;
                    break;
                }
            }
            
            // Save current team at the position where lowestTeam will move to
            int oldRanking = team.ranking;
            
            // Unfreeze this problem - process all frozen submissions for this problem
            ProblemStatus& ps = team.problems[smallestProblem];
            int processedCount = ps.frozenSubmissions;
            
            // Find where frozen submissions start
            int totalSubs = ps.submissions.size();
            int startIdx = totalSubs - processedCount;
            
            for (int i = startIdx; i < totalSubs; i++) {
                const Submission& sub = ps.submissions[i];
                if (!ps.solved) {
                    if (sub.status == "Accepted") {
                        ps.solved = true;
                        ps.solveTime = sub.time;
                    } else {
                        ps.wrongAttempts++;
                    }
                }
            }
            ps.frozenSubmissions = 0;
            
            // Before updating, find which team is at each ranking
            map<int, string> rankToTeam;
            for (const auto& name : teamNames) {
                rankToTeam[teams[name].ranking] = name;
            }
            
            updateRankings();
            int newRanking = team.ranking;
            
            if (newRanking < oldRanking) {
                // The team that was at newRanking is the replaced team
                string replacedTeam = rankToTeam[newRanking];
                cout << lowestTeam << " " << replacedTeam << " " 
                     << team.solvedCount << " " << team.penaltyTime << "\n";
            }
        }
        
        printScoreboard();
        frozen = false;
    }
    
    void queryRanking(const string& name) {
        if (teams.find(name) == teams.end()) {
            cout << "[Error]Query ranking failed: cannot find the team.\n";
            return;
        }
        cout << "[Info]Complete query ranking.\n";
        if (frozen) {
            cout << "[Warning]Scoreboard is frozen. The ranking may be inaccurate until it were scrolled.\n";
        }
        cout << name << " NOW AT RANKING " << teams[name].ranking << "\n";
    }
    
    void querySubmission(const string& teamName, const string& problemName, const string& statusName) {
        if (teams.find(teamName) == teams.end()) {
            cout << "[Error]Query submission failed: cannot find the team.\n";
            return;
        }
        
        cout << "[Info]Complete query submission.\n";
        
        const Team& team = teams[teamName];
        const Submission* lastSub = nullptr;
        
        for (char p = 'A'; p < 'A' + problemCount; p++) {
            if (problemName != "ALL" && p != problemName[0]) continue;
            
            auto it = team.problems.find(p);
            if (it != team.problems.end()) {
                const ProblemStatus& ps = it->second;
                for (const Submission& sub : ps.submissions) {
                    if (statusName == "ALL" || sub.status == statusName) {
                        if (lastSub == nullptr || sub.submissionId > lastSub->submissionId) {
                            lastSub = &sub;
                        }
                    }
                }
            }
        }
        
        if (lastSub == nullptr) {
            cout << "Cannot find any submission.\n";
        } else {
            cout << teamName << " " << lastSub->problem << " " 
                 << lastSub->status << " " << lastSub->time << "\n";
        }
    }
    
    void end() {
        cout << "[Info]Competition ends.\n";
    }
};

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);
    
    ICPCSystem system;
    string line;
    
    while (getline(cin, line)) {
        istringstream iss(line);
        string cmd;
        iss >> cmd;
        
        if (cmd == "ADDTEAM") {
            string name;
            iss >> name;
            system.addTeam(name);
        } else if (cmd == "START") {
            string duration, problem;
            int dur, prob;
            iss >> duration >> dur >> problem >> prob;
            system.startCompetition(dur, prob);
        } else if (cmd == "SUBMIT") {
            char problem;
            string by, teamName, with, status, at;
            int time;
            iss >> problem >> by >> teamName >> with >> status >> at >> time;
            system.submit(problem, teamName, status, time);
        } else if (cmd == "FLUSH") {
            system.flush();
        } else if (cmd == "FREEZE") {
            system.freeze();
        } else if (cmd == "SCROLL") {
            system.scroll();
        } else if (cmd == "QUERY_RANKING") {
            string name;
            iss >> name;
            system.queryRanking(name);
        } else if (cmd == "QUERY_SUBMISSION") {
            string teamName;
            iss >> teamName;
            
            string where, part1, and_word, part2;
            iss >> where >> part1 >> and_word >> part2;
            
            string problemName, statusName;
            size_t pos1 = part1.find('=');
            problemName = part1.substr(pos1 + 1);
            
            size_t pos2 = part2.find('=');
            statusName = part2.substr(pos2 + 1);
            
            system.querySubmission(teamName, problemName, statusName);
        } else if (cmd == "END") {
            system.end();
            break;
        }
    }
    
    return 0;
}
