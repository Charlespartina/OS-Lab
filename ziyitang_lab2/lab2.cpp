/*
 *	Operating System Lab 2
 *	Created by Ziyi Tang
 *	Scheduler
 */

//#include <bits/stdc++.h>
#include <iostream>
#include <cstdio>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <cmath>
#include <algorithm>
#include <list>
#include <vector>
#include <set>
#include <map>
#include <stack>
#include <queue>
typedef long long ll;
using namespace std;

queue<ll> randomNums;
bool verbose;

// Process Class
struct Process {
	int idx_;
	int A_,B_,C_,M_;
	int states[3]; // Ready, Runninng, Blocked
	int running_time; // Keep track of running time
	int cur_state; // Current State
	int cur_cpu_burst; // Current burst time
	int io_time; // Total io time
	int waiting_time; // Total waiting time
	int finish_time; // Finishing time
	Process (int idx, int A, int B, int C, int M) : 
		A_(A), B_(B), C_(C), M_(M), cur_state(-1), io_time(0), waiting_time(0), running_time(0), idx_(idx), finish_time(-1), cur_cpu_burst(0) {
		memset(states,0,sizeof(states));
	}
	void printTuple() const {
		printf("(%d %d %d %d)", A_, B_, C_, M_);
	}

	// A process is small if it has earlier starting time and input time
	bool operator == (const Process& tmp) const {
		return A_ == tmp.A_ && idx_ == tmp.idx_;
	}
	bool operator < (const Process& tmp) const {
		if(A_ == tmp.A_) 
			return idx_ < tmp.idx_;
		return A_ < tmp.A_;
	}
	bool operator > (const Process& tmp) const {
		if(A_ == tmp.A_) 
			return idx_ > tmp.idx_;
		return A_ > tmp.A_;
	}
};

// Summary Data class
struct SummaryData {
	int finish_time;
	// Util
	double cpu_util_time;
	double io_util_time;
	double throughput;
	// Turnaround and wait
	double avg_turn_time;
	double avg_wait_time;
	SummaryData() : finish_time(0), cpu_util_time(0), io_util_time(0), throughput(0), avg_wait_time(0), avg_turn_time(0) {}
	void gather(int sz, int timestamp){
		finish_time = timestamp-1;
		cpu_util_time /= finish_time;
		io_util_time /= finish_time;
		avg_wait_time /= sz;
		avg_turn_time /= sz;
		throughput = (double)sz*100/finish_time;
	}
	void printSummary() const {
		printf("Summary Data:\n");
		printf("    Finishing time: %d\n", finish_time);
		printf("    CPU Utilization: %.6lf\n", cpu_util_time);
		printf("    I/O Utilization: %.6f\n", io_util_time);
		printf("    Throughput: %.6f processes per hundred cycles\n", throughput);
		printf("    Average turnaround time: %.6f\n", avg_turn_time);
		printf("    Average waiting time: %.6f\n\n", avg_wait_time);
	}
};

// Random Mod
int randomMod(int B){
	int random = (int)rand();
	return 1+random%B;
}
int randomMod_feed(int B){
	if(randomNums.empty())
		return randomMod(B);
	ll num = randomNums.front(); randomNums.pop();
	// cout << "random " << num << endl;
	return (int)(1+num%B);
}

// Print Info
void printInfo(string algo, const vector<Process>& processes, const SummaryData& summary){
	printf("The scheduling algorithm used was %s\n\n", algo.c_str());
	for(int i = 0; i < processes.size(); i++){
		printf("Process %d:\n", i);
		const Process& now = processes[i];
		printf("    (A,B,C,M)=(%d,%d,%d,%d)\n", now.A_, now.B_, now.C_, now.M_);
		printf("    Finishing time: %d\n", now.finish_time);
		printf("    Turnaround time: %d\n", now.finish_time-now.A_);
		printf("    I/O time: %d\n",now.io_time);
		printf("    Waiting time: %d\n\n",now.waiting_time);
	}
	summary.printSummary();
}

// First Come First Serve
void FCFS(vector<Process> allProcesses, string name){
	// Priority Queue for Ready Time and Process Index
	priority_queue<pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > readyQueue;
	// Init SummaryData
	SummaryData summary = SummaryData();
	// No process is running right now
	int cur_process = -1;
	// Keep track of the unstarted processes
	int pt = 0;
	int timestamp = 0;
	while(1){
		// Flag indicating that every process is terminated
		bool TERMINATED = true;
		// Check status for all processes
		// Make one move
		printf("Before cycle   %d:", timestamp);
		bool IO = false;
		for(int i = 0; i < allProcesses.size(); i++){
			Process* now = &allProcesses[i];
			//cout << "sum: " << now->states[0] << " " << now->states[1] << " " << now->states[2] << endl;
			cout << "    ";
			// Not Created
			if(now->cur_state == -1){
				cout << "unstarted 0";
				TERMINATED = false;
				continue;
			}
			// Terminated
			if(now->cur_state == 3){
				cout << "terminated 0";
				if(now->finish_time == -1){
					now->finish_time = timestamp-1;
					summary.avg_turn_time += now->finish_time-now->A_;
				}
				continue;
			}

			// If process is running or blocking
			TERMINATED = false;
			int cur_state = now->cur_state;
			if(cur_state == 1){
				cout << "running "; cout << (now->states)[cur_state]--;
				now->running_time++;
				summary.cpu_util_time++; // Utilize CPU
				if(now->states[cur_state] == 0){
					// The current cpu burst time is reached
					if(now->running_time == allProcesses[i].C_){
						// Terminate this process
						now->cur_state = 3;
					} else {
						// Blocking
						now->cur_state = 2;
					}
					// No process is running
					cur_process = -1;
				}
			} else if(cur_state == 2){
				cout << "blocked "; cout << (now->states)[cur_state]--;
				now->io_time++;
				IO = true; // Some process is waiting for IO
				if(now->states[cur_state] == 0){
					// Go to the ready list
					now->cur_state = 0;
					readyQueue.push(make_pair(timestamp, now->idx_));
				}
			} else{
				// Otherwise, the process is in ready state
				now->waiting_time++;
				summary.avg_wait_time++;
				cout << "ready 0";
			}
		}
		if(IO) summary.io_util_time++; // Utilize I/O

		// All processes are terminated
		if(TERMINATED == true) {
			cout << endl;
			int sz = allProcesses.size();
			summary.gather(sz, timestamp);
			printInfo(name, allProcesses, summary);
			break;
		}

		// Add processes into ready list if they are created at this time
		while(pt < allProcesses.size() && allProcesses[pt].A_ == timestamp){
			// cout << "add" << allProcesses[pt].A_ << " " << allProcesses[pt].B_ << " " << allProcesses[pt].C_ << " " << allProcesses[pt].M_ << endl;
			allProcesses[pt].cur_state = 0;
			readyQueue.push(make_pair(timestamp, allProcesses[pt].idx_));
			pt++;
		}

		// No process is running, add a process that is ready
		if(cur_process == -1){
			// No process is ready
			if(!readyQueue.empty()) {
				int nowIdx = readyQueue.top().second; readyQueue.pop();
				Process* now = &allProcesses[nowIdx];
				// cout << now.A_ << " " << now.B_ << " " << now.C_ << " " << now.M_ << endl;
				int cpu_burst = min(now->C_-now->running_time, randomMod_feed(now->B_));
				int io_burst = now->M_ * cpu_burst;
				now->states[1] = cpu_burst;
				now->states[2] = io_burst;
				now->cur_state = 1;
				cur_process = nowIdx;
			}
		}
		cout << endl;
		timestamp++;
	}
}

// Round Robbin
// Similar to FCFS, with preemption
void RR(vector<Process> allProcesses, string name, int quantum){
	// Priority Queue for Ready Time and Process Index
	priority_queue<pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > readyQueue;
	// Init SummaryData
	SummaryData summary = SummaryData();
	// No process is running right now
	int cur_process = -1;
	// Keep track of the unstarted processes
	int pt = 0;
	int timestamp = 0;
	// Keep track of preempt status
	vector<int> preempt(allProcesses.size(), quantum);
	while(1){
		// Flag indicating that every process is terminated
		bool TERMINATED = true;
		// Check status for all processes
		// Make one move
		printf("Before cycle   %d:", timestamp);
		bool IO = false;
		for(int i = 0; i < allProcesses.size(); i++){
			Process* now = &allProcesses[i];
			// cout << "sum: " << now->states[0] << " " << now->states[1] << " " << now->states[2] << endl;
			cout << "    ";
			// Not Created
			if(now->cur_state == -1){
				cout << "unstarted 0";
				TERMINATED = false;
				continue;
			}
			// Terminated
			if(now->cur_state == 3){
				cout << "terminated 0";
				if(now->finish_time == -1){
					now->finish_time = timestamp-1;
					summary.avg_turn_time += now->finish_time-now->A_;
				}
				continue;
			}

			// If process is running or blocking
			TERMINATED = false;
			int cur_state = now->cur_state;
			if(cur_state == 1){
				preempt[now->idx_] = min(preempt[now->idx_], now->states[cur_state]);
				cout << "running "; cout << preempt[now->idx_]--;
				(now->states)[cur_state]--;
				now->running_time++;
				summary.cpu_util_time++; // Utilize CPU
				if(now->states[cur_state] == 0){
					// The current cpu burst time is reached
					if(now->running_time == allProcesses[i].C_){
						// Terminate this process
						now->cur_state = 3;
					} else {
						// Blocking
						now->cur_state = 2;
					}
					// Restore the preempt indicator
					preempt[now->idx_] = quantum;
					// No process is running
					cur_process = -1;
				}
				// Preempt the process
				if(preempt[now->idx_] == 0){
					preempt[now->idx_] = quantum;
					cur_process = -1;
					now->cur_state = 0;
					readyQueue.push(make_pair(timestamp, now->idx_));
				}
			} else if(cur_state == 2){
				cout << "blocked "; cout << (now->states)[cur_state]--;
				now->io_time++;
				IO = true; // Some process is waiting for IO
				if(now->states[cur_state] == 0){
					// Go to the ready list
					now->cur_state = 0;
					readyQueue.push(make_pair(timestamp, now->idx_));
				}
			} else{
				// Otherwise, the process is in ready state
				now->waiting_time++;
				summary.avg_wait_time++;
				cout << "ready 0";
			}
		}
		if(IO) summary.io_util_time++; // Utilize I/O

		// All processes are terminated
		if(TERMINATED == true) {
			cout << endl;
			int sz = allProcesses.size();
			summary.gather(sz, timestamp);
			printInfo(name, allProcesses, summary);
			break;
		}

		// Add processes into ready list if they are created at this time
		while(pt < allProcesses.size() && allProcesses[pt].A_ == timestamp){
			// cout << "add" << allProcesses[pt].A_ << " " << allProcesses[pt].B_ << " " << allProcesses[pt].C_ << " " << allProcesses[pt].M_ << endl;
			allProcesses[pt].cur_state = 0;
			readyQueue.push(make_pair(timestamp, allProcesses[pt].idx_));
			pt++;
		}

		// No process is running, add a process that is ready
		if(cur_process == -1){
			// No process is ready
			if(!readyQueue.empty()) {
				int nowIdx = readyQueue.top().second; readyQueue.pop();
				Process* now = &allProcesses[nowIdx];
				// cout << now.A_ << " " << now.B_ << " " << now.C_ << " " << now.M_ << endl;
				if(now->states[1] == 0){
					// Not enqueued by preemption
					int cpu_burst = min(now->C_-now->running_time, randomMod_feed(now->B_));
					int io_burst = now->M_ * cpu_burst;
					now->states[1] = cpu_burst;
					now->states[2] = io_burst;
					now->cur_cpu_burst = cpu_burst;
				}
				now->cur_state = 1;
				cur_process = nowIdx;
			}
		}
		cout << endl;
		timestamp++;
	}
}

// Uniprogramming
void UNI(vector<Process> allProcesses, string name){
	// Priority Queue for Ready Time and Process Index
	priority_queue<pair<int,int>, vector<pair<int,int> >, greater<pair<int,int> > > readyQueue;
	// Init SummaryData
	SummaryData summary = SummaryData();
	// No process is running right now
	int cur_process = -1;
	// Keep track of the unstarted processes
	int pt = 0;
	int timestamp = 0;
	while(1){
		// Flag indicating that every process is terminated
		bool TERMINATED = true;
		// Check status for all processes
		// Make one move
		printf("Before cycle   %d:", timestamp);
		bool IO = false;
		for(int i = 0; i < allProcesses.size(); i++){
			Process* now = &allProcesses[i];
			//cout << "sum: " << now->states[0] << " " << now->states[1] << " " << now->states[2] << endl;
			cout << "    ";
			// Not Created
			if(now->cur_state == -1){
				cout << "unstarted 0";
				TERMINATED = false;
				continue;
			}
			// Terminated
			if(now->cur_state == 3){
				cout << "terminated 0";
				if(now->finish_time == -1){
					now->finish_time = timestamp-1;
					summary.avg_turn_time += now->finish_time-now->A_;
				}
				continue;
			}

			// If process is running or blocking
			TERMINATED = false;
			int cur_state = now->cur_state;
			if(cur_state == 1){
				cout << "running "; cout << (now->states)[cur_state]--;
				now->running_time++;
				summary.cpu_util_time++; // Utilize CPU
				if(now->states[cur_state] == 0){
					// The current cpu burst time is reached
					if(now->running_time == allProcesses[i].C_){
						// Terminate this process
						now->cur_state = 3;
						cur_process = -1;
					} else {
						// Blocking
						now->cur_state = 2;
					}
				}
			} else if(cur_state == 2){
				cout << "blocked "; cout << (now->states)[cur_state]--;
				now->io_time++;
				IO = true; // Some process is waiting for IO
				if(now->states[cur_state] == 0){
					// Run another round
					int cpu_burst = min(now->C_-now->running_time, randomMod_feed(now->B_));
					int io_burst = now->M_ * cpu_burst;
					now->states[1] = cpu_burst;
					now->states[2] = io_burst;
					now->cur_state = 1;
				}
			} else{
				// Otherwise, the process is in ready state
				now->waiting_time++;
				summary.avg_wait_time++;
				cout << "ready 0";
			}
		}
		if(IO) summary.io_util_time++; // Utilize I/O

		// All processes are terminated
		if(TERMINATED == true) {
			cout << endl;
			int sz = allProcesses.size();
			summary.gather(sz, timestamp);
			printInfo(name, allProcesses, summary);
			break;
		}

		// Add processes into ready list if they are created at this time
		while(pt < allProcesses.size() && allProcesses[pt].A_ == timestamp){
			// cout << "add" << allProcesses[pt].A_ << " " << allProcesses[pt].B_ << " " << allProcesses[pt].C_ << " " << allProcesses[pt].M_ << endl;
			allProcesses[pt].cur_state = 0;
			readyQueue.push(make_pair(timestamp, allProcesses[pt].idx_));
			pt++;
		}

		// Some process is terminated
		if(cur_process == -1){
			// No process is ready
			if(!readyQueue.empty()) {
				int nowIdx = readyQueue.top().second; readyQueue.pop();
				Process* now = &allProcesses[nowIdx];
				// cout << now.A_ << " " << now.B_ << " " << now.C_ << " " << now.M_ << endl;
				int cpu_burst = min(now->C_-now->running_time, randomMod_feed(now->B_));
				int io_burst = now->M_ * cpu_burst;
				now->states[1] = cpu_burst;
				now->states[2] = io_burst;
				now->cur_state = 1;
				cur_process = nowIdx;
			}
		}
		cout << endl;
		timestamp++;
	}
}

// Shortest Job First
void SJF(vector<Process> allProcesses){
	
}

int main(){
	vector<Process> allProcesses;
	// Input the process tuples
	int n;
	cin >> n;
	string line;
	getline(cin,line);
	stringstream ss(line);
	string tmp = "";
	for(int i = 0; i < n; i++){
		vector<int> nums;
		for(int j = 0; j < 4; j++){
			ss >> tmp;
			if(tmp[0] == '(')
				tmp.erase(tmp.begin());
			if(tmp[tmp.size()-1] == ')')
				tmp.pop_back();
			int val = 0;
			istringstream ss2(tmp);
			ss2 >> val;
			nums.push_back(val);
		}
		allProcesses.push_back(Process(i, nums[0], nums[1], nums[2], nums[3]));
		//cout << nums[0] << " " << nums[1] << " " << nums[2] << " " << nums[3] << endl;
	}
	cout << "The original input was: " << n;
	for(int i = 0; i < allProcesses.size(); i++){
		cout << " ";
		allProcesses[i].printTuple();
	}
	cout << endl;
	stable_sort(allProcesses.begin(), allProcesses.end());
	cout << "The (sorted) input is: " << n;
	for(int i = 0; i < allProcesses.size(); i++){
		cout << " ";
		allProcesses[i].printTuple();
		allProcesses[i].idx_ = i; // Re-indexing
	}
	cout << endl;
	printf("\nThis detailed printout gives the state and remaining burst for each process\n");
	freopen("random-numbers", "r", stdin);
	int num = 0;
	while(cin >> num){
		randomNums.push(num);
	}
	fclose(stdin);
	// FCFS(allProcesses, "First Come First Served");
	// RR(allProcesses, "Round Robbin", 2);
	// UNI(allProcesses, "Uniprogramming");
	return 0;
}