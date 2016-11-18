#include<iostream>
#include<cstdio>
#include<cstring>
#include<cstdlib>
#include<cmath>
#include<vector>
#include<set>
#include<map>
#include<queue>
#include<stack>
#include<sstream>
#include<algorithm>

#define CLAIM 0
#define HAS 1
#define REM 2

#define DEBUG if(0)
#define OPT 0
#define BAN 1
using namespace std;
typedef vector<pair<string, vector<int> > > vpsv; 
typedef vector<vpsv> vvpsv;
typedef vector<int> vi;
typedef vector<vi> vvi;
typedef vector<vvi> vvvi;

struct Process{
	int idx;
	int time;
	int wait;
	bool aborted;
	bool blocked;
	bool terminated;
	Process():idx(0),time(0),wait(0),aborted(false),blocked(false),terminated(false) {}
};
// This function serves to check if there is a deadlock in a given cycle
bool checkDL(vvpsv& coms, vector<int>& resources){
	for(int i = 1; i < coms.size(); i++){
		if(coms[i].size() == 0) continue;
		string com = (coms[i].begin())->first; 
		int type = ((coms[i].begin())->second)[1]; 
		int number = ((coms[i].begin())->second)[2]; 
		if(com != "request") return false;
		if(resources[type] >= number) return false;
	}
	return true;
}

// This function resolves deadlock by aborting one task and returning all its resources
void resolveDL(vvpsv& coms, vi& resources, vvvi& matrix, vector<Process>& vp, vi& BlockedList){
	for(int i = 1; i < coms.size(); i++){
		if(coms[i].begin() != coms[i].end()){
			DEBUG printf("Abort task %d\n", i);
			// Return every resources
			for(int j = 1; j < resources.size(); j++){
				DEBUG printf("Return resource %d for %d unit\n", j, matrix[i][HAS][j]);
				resources[j] += matrix[i][HAS][j];
				matrix[i][REM][j] += matrix[i][HAS][j];
				matrix[i][HAS][j] = 0;
			}
			// Delete the task operations
			coms[i].clear();
			// Delete the task from Blocked List
			for(int k = 0; k < BlockedList.size(); k++) if(BlockedList[k] == i)
				BlockedList.erase(BlockedList.begin()+k);
			vp[i].aborted = true;
			break;
		}
	}
}
// This function check if a given state is a safe state
bool isSafe(vvvi matrix, vi resources, int active, vector<Process> tvp){
	while(1){
		int flag = 0; // Indicate if there is a job that can be completed
		for(int i = 1; i < matrix.size(); i++){
			if(tvp[i].terminated || tvp[i].aborted) continue;
			// Check if all types of resources can be satisfied for a given task's claim
			int can = 1;
			for(int j = 1; j < resources.size(); j++){
				if(resources[j] < matrix[i][REM][j]){
					can = 0;
					break ;
				}
			}
			if(!can) continue;
			else {
				// We can complete this task and so release all resources from it
				active--;
				flag = 1;
				for(int j = 1; j < resources.size(); j++){
					resources[j] += matrix[i][HAS][j];
					matrix[i][HAS][j] = 0;
					matrix[i][REM][j] = matrix[i][CLAIM][j];
				}
				tvp[i].terminated = true;
			}
		}
		if(!flag) break;
	}
	DEBUG {
		if(active) cout << "Unsafe State" << endl;
		else cout << "Safe State" << endl;
	}
	if(active) return false;
	return true;
}
void process(	int mode,
		vvpsv coms,
		vvvi matrix,
		vi resources,
		vector<Process>& vp
	       ){
	vector<int> blockedList;
	int active = coms.size()-1;
	int counter = 0;
	while(1){
		DEBUG cout << "\nCycle: " << counter++ << endl;
		DEBUG {
			cout << "Resource remain "; for(int i = 1; i < resources.size(); i++) cout << resources[i] << " "; cout << endl;
		}
		// Go to next cycle
		int flag = 0; // Indicate if there's still process not terminated
		int dl = 0; // Indicate if there's deadlock in this cycle
		// Check if there's task in blocked list
		map<int,int> tmpRelease; // Hold the released resources until the end of the cycle
		vector<int> visited(coms.size()+1, 0); // Mark as visited if a task is processed in this cycle
		DEBUG cout << "BlockedList Size " <<  blockedList.size() << endl;
		for(int i = 0; i < blockedList.size(); i++){
			flag = 1;
			int task = blockedList[i];
			visited[task] = 1;
			pair<string,vector<int> > now = *coms[task].begin();	
			int type = (now.second)[1];
			int number = (now.second)[2];
			vp[task].time++;
			vp[task].wait++;
			// For optimistic case
			if(mode == OPT){
				if(resources[type]-number >= 0){
					vp[task].blocked = 0;
					blockedList.erase(blockedList.begin()+i);
					coms[task].erase(coms[task].begin());
					resources[type]-=number;
					matrix[task][HAS][type]+=number;
					matrix[task][REM][type]-=number;
					i--;
					DEBUG printf("From blocked List: task %d's request for %d units of type %d is granted\n", task, number, type); 
				} else {
					DEBUG printf("From blocked List: task %d's request for %d units of type %d is not granted\n", task, number, type); 	
				}
			}
			// For Banker case
			else {
				vvvi tmpMatrix = matrix;
				vi tmpResources = resources;
				tmpMatrix[task][HAS][type]+=number;
				tmpMatrix[task][REM][type]-=number;
				tmpResources[type]-=number;
				// Check if the next state is safe
				if(isSafe(tmpMatrix, tmpResources, active, vp)){
					DEBUG printf("From Blocked List: request success for %d\n", task);			
					matrix = tmpMatrix;
					vp[task].blocked = 0;
					resources = tmpResources;
					blockedList.erase(blockedList.begin()+i);
					coms[task].erase(coms[task].begin());
					i--;	
				} else {
					DEBUG printf("From Blocked List: request failed for %d\n", task);					
				}
			}
		}
		// Check remaining tasks
		DEBUG cout << "Blocked List Processed Over" << endl;
		for(int i = 1; i < coms.size(); i++){
			if(coms[i].begin() != coms[i].end()){
				flag = 1;
				pair<string,vector<int> >& now = *coms[i].begin();
				string com = now.first;
				int& task = (now.second)[0];
				int& type = (now.second)[1];
				int& number = (now.second)[2];
				if(visited[task] == 1) continue;
				visited[task] = 1;
				DEBUG cout << "-----" << com << " " << task << " " << type << " " << number << "-----" << endl;
				vp[task].time++;
				if(com == "initiate"){
					if(mode == BAN && number > resources[type]) {
						DEBUG printf("Task %d is aborted. The claim is too big\n", task);
						vp[task].aborted = true;
						coms[task].clear();
						active--;
						continue;
					} else {
						matrix[task][CLAIM][type] = matrix[task][REM][type] = number;
						DEBUG printf("initiate %d\n", task);
					}
				} else if (com == "request"){
					// For optimistic case, we assign the resources if they are available
					if(mode == OPT){
						if(resources[type]-number < 0){
							vp[task].blocked = 1;
							blockedList.push_back(task);
							DEBUG printf("request failed for %d, blocked\n", task);
							continue; // Skip the "erase" part
						} else {
							resources[type]-=number;
							matrix[task][HAS][type] += number;
							matrix[task][REM][type] -= number;
							DEBUG printf("request success for %d\n", task);
						}
					} else {
						// Check if the request violates the claim
						if(number > matrix[task][REM][type]){
							DEBUG cout << "Invalid Request, Aborted" << endl;
							// Abort the task
							vp[task].aborted = true;
							active--;
							for(int j = 1; j < resources.size(); j++){
								matrix[task][REM][j] = matrix[task][CLAIM][j];
								tmpRelease[j] += matrix[task][HAS][j];
								matrix[task][HAS][j] = 0;
							}
							coms[task].clear();
							continue;
						}
						DEBUG printf("Task %d tries to request %d unit of type %d from banker\n", task, number, type);
						vvvi tmpMatrix = matrix;
						vi tmpResources = resources;
						tmpMatrix[task][HAS][type]+=number;
						tmpMatrix[task][REM][type]-=number;
						tmpResources[type]-=number;
						// Check if the next state is safe
						if(isSafe(tmpMatrix, tmpResources, active, vp)){
							DEBUG printf("request success for %d\n", task);
							matrix = tmpMatrix;
							resources = tmpResources;
						} else {
							DEBUG printf("request failed for %d, Blocked\n", task);
							blockedList.push_back(task);
							vp[task].blocked = 1;
							continue; // Skip the "erase" part
						}
					}
				} else if (com == "release"){
					tmpRelease[type]+=number;
					matrix[task][HAS][type] -= number;
					matrix[task][REM][type] += number;			
					DEBUG printf("release success for %d\n", task);
				} else if (com == "compute"){
					DEBUG printf("compute for %d, %d remained\n", task, type);
					type--;
					if(type > 0)
						continue; // Skip the "erase" part
				} else {
					active--; // Terminated
					vp[task].terminated = true;
					// Return all resources
					for(int j = 1; j < resources.size(); j++){
						tmpRelease[j] += matrix[task][HAS][j];
						matrix[task][HAS][j] = 0;
						matrix[task][REM][j] = matrix[task][CLAIM][j];
					}
				}
				coms[i].erase(coms[i].begin());
			}
		}		
		DEBUG cout << "Active Process: " << active << endl;
		if(mode == OPT){
			// Check Deadlock
			while(blockedList.size() == active && checkDL(coms, resources)){
				DEBUG cout << "Deadlock Detected" << endl;
				resolveDL(coms, resources, matrix, vp, blockedList);
				active-=1;
			}
		}
		// Give back released resources
		for(map<int,int>::iterator it = tmpRelease.begin(); it != tmpRelease.end(); it++){
			resources[it->first]+=it->second;
		}
		tmpRelease.clear();
		if(!flag) return ;
	}

}
void printResult(int mode, vector<Process> vp){
	if(mode == OPT)
		printf("FIFO\n");
	else printf("BANKER\n");
	int sum_wait = 0;
	int sum_time = 0;
	for(int i = 1; i < vp.size(); i++){
		printf("Task %d     ", i);
		if(vp[i].aborted == true)
			printf("aborted\n");
		else{
			printf("%d  %d  %d%%\n", vp[i].time-1, vp[i].wait, vp[i].wait*100/(vp[i].time-1));
			sum_time += vp[i].time-1;
			sum_wait += vp[i].wait;
		}
	}
	printf("Total:     %d  %d  %d%%\n", sum_time, sum_wait, (int)(sum_wait*100/sum_time+1e-7));

}
int main(int num, char* args[]){
	int taskNum, resNum; // Number of tasks, number of resources
	vi resources; // Keep track of the amount of each resource
	vector<Process> vp; // All processes
 	vpsv pre; // Operation: (task#, type of resource, number)
	vvvi matrix; // An table of resources allocation states
	vvpsv coms; // Records operations for each task
	if(num < 2) {
		cout << "Please specify file name" << endl;
		return 0;
	}
	freopen(args[1], "r", stdin);
	DEBUG cout << "File: " << args[1] << endl;
	scanf("%d %d", &taskNum, &resNum);
	DEBUG cout << taskNum << " " << resNum << endl;
	resources.assign(resNum+1, 0); // Assign 0 to all resources
	coms.assign(taskNum+1, pre);
	vp.assign(taskNum+1, Process());
	for(int i = 1; i <= resNum; i++)
		scanf("%d", &resources[i]);
	string com;
	// Each row has 3 columns, each column has resNum of 0
	matrix.assign(taskNum+1, vector<vi>(3, vi(resNum+1,0)));
	// Record all operations
	
	while(cin >> com){
		int task, type, number;
		cin >> task >> type >> number;
		vector<int> tmp;
		tmp.push_back(task);
		tmp.push_back(type);
		tmp.push_back(number);
		coms[task].push_back(make_pair(com, tmp));
	}
	fclose(stdin);
	// Optimistic Case
	process(OPT, coms, matrix, resources, vp);
	printResult(OPT,vp);
	cout << endl;	
	// Banker Case
	vp.assign(taskNum+1, Process());
	process(BAN, coms, matrix, resources, vp);
	printResult(BAN,vp);
	return 0;
}
