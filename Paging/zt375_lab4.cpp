#include <iostream>
#include <vector>
#include <queue>
#include <map>
#include <list>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <cstring>
#include <algorithm>

#define DEBUG if(0)
#define MAXINT 2147483647
#define Q 3
#define LIFO 0
#define RANDOM 1
#define LRU 2
using namespace std;

typedef pair<int,int> pi;
typedef long long ll;
typedef vector<ll> vll;
typedef vector<double> vd;
typedef vector<vd> vvd;
int timer = 0;
ll M; // machine size
ll P; // page size
ll S; // process size
ll J; // job mix
ll N; // num of references
string R; // Replacement Algorithm
queue<ll> random_number;
vector<pi> frameTable;
int lastPageFrame; // last loaded page
map<pi,int> pageToFrame;
map<pi,int> start_resi_time;
int numFrames;

// LRU List
list<pi> LRU_list;
map<pi, list<pi>::iterator> LRU_map;

struct Process {
	int idx;
	int next_word;
	int q;
	int ref;
	int num_of_fault;
	int total_resi_time;
	int start_resi_time;
	int num_of_evic;
	double A,B,C,D;
	Process(): idx(0), q(Q), ref(0), next_word(0), num_of_fault(0), total_resi_time(0), num_of_evic(0), A(0),B(0),C(0),D(0){}
	Process(int idx_) {
		idx = idx_;
		q = Q;
		ref = N;
		next_word = 111*idx%S;
		num_of_fault = 0;
		total_resi_time = 0;
		num_of_evic = 0;
		A=B=C=D=0.0;
	}
};

vector<Process> jobs;

ll getRandom(){
	ll now = random_number.front();random_number.pop();
	return now;
}

// This function handles eviction
void evict(int replaceFrame, int idx, int page){
	pi lastP = frameTable[replaceFrame];
	pageToFrame.erase(lastP);
	frameTable[replaceFrame] = make_pair(idx, page);
	pageToFrame[make_pair(idx, page)] = replaceFrame;
	// Increment the num of eviction
	jobs[lastP.first].num_of_evic++;
	jobs[lastP.first].total_resi_time += (timer-start_resi_time[make_pair(lastP.first, lastP.second)]);
	DEBUG printf("Fault. Evicting page %d of %d from frame %d\n", lastP.second, lastP.first, replaceFrame);
}

// This function finds a location for a given process
// And does eviction if neccessary
void findLoc(int idx, int page){
	// Start resi
	start_resi_time[make_pair(idx,page)] = timer;
	for(int i = numFrames-1; i >= 0; i--){
		pi& now = frameTable[i];
		// Empty frame
		if(now.first == -1){
			now.first = idx;
			now.second = page;
			pageToFrame[make_pair(idx,page)] = i;
			lastPageFrame = i;
			DEBUG printf("Fault. Using free frame %d\n", i);
			LRU_list.push_front(make_pair(idx,page));
			LRU_map[make_pair(idx,page)] = LRU_list.begin();
			return ;
		}
	}
	// If no empty frame is available
	if(R == "lifo"){
		evict(lastPageFrame, idx, page);
	} else if(R == "random"){
		ll ran = getRandom();
		// DEBUG printf("%d uses random number %lld ", idx, ran);
		evict(ran%numFrames, idx, page);
	} else {
		pi last = LRU_list.back();
		LRU_list.pop_back();
		LRU_map.erase(make_pair(last.first, last.second));
		evict(pageToFrame[make_pair(last.first, last.second)], idx, page);
	}
	LRU_list.push_front(make_pair(idx,page));
	LRU_map[make_pair(idx,page)] = LRU_list.begin();
}
void process(){
	int D = 0;
	if(J == 1){
		D = 1;
	} else D = 4;
	queue<int> qq;
	jobs.push_back(Process());
	for(int i = 1; i <= D; i++){
		jobs.push_back(Process(i));
		qq.push(i);
	}
	if(J == 1){
		jobs[1].A = 1;
		jobs[1].B = 0;
		jobs[1].C = 0;
	}
	if(J == 2){
		for(int i = 1; i <= D; i++){
			jobs[i].A = 1;
			jobs[i].B = 0;
			jobs[i].C = 0;
		}
	}
	if(J == 3){
		for(int i = 1; i <= D; i++){
			jobs[i].A = 0;
			jobs[i].B = 0;
			jobs[i].C = 0;
		}
	}
	if(J == 4){
		jobs[1].A = 0.75;
		jobs[1].B = 0.25;
		jobs[1].C = 0;
		jobs[2].A = 0.75;
		jobs[2].B = 0;
		jobs[2].C = 0.25;
		jobs[3].A = 0.75;
		jobs[3].B = 0.125;
		jobs[3].C = 0.125;
		jobs[4].A = 0.5;
		jobs[4].B = 0.125;
		jobs[4].C = 0.125;
	}
	while(!qq.empty()){
		Process& now = jobs[qq.front()]; qq.pop();
		while(now.q && now.ref){
			timer++;
			int page = now.next_word/P;
			DEBUG printf("%d references word %d (page %d) at time %d: ", now.idx, now.next_word, page, timer);
			// Page Hit
			if(pageToFrame.find(make_pair(now.idx,page)) != pageToFrame.end()){
				list<pi>::iterator it = LRU_map[make_pair(now.idx, page)];
				LRU_list.splice(LRU_list.begin(), LRU_list, it);
				DEBUG printf("Hit in frame %d\n", pageToFrame[make_pair(now.idx,page)]);
			} else {
				now.num_of_fault++;
				findLoc(now.idx, page);
			}
			now.q--;
			now.ref--;
			ll ran = getRandom();
			// DEBUG printf("%d uses random number %lld\n", now.idx, ran);
			double y = ran / ((double)MAXINT + 1.0);
			if(y < now.A){
				now.next_word  = (now.next_word+1)%S;
			} else if(y < now.A+now.B){
				now.next_word  = (now.next_word-5+S)%S;
			} else if(y < now.A+now.B+now.C){
				now.next_word  = (now.next_word+4)%S;
			} else{
				now.next_word  = getRandom()%S;
			}
		}
		if(now.ref) {
			now.q = Q;
			qq.push(now.idx);
		}
		// for(int i = 0; i < numFrames; i++){
		// 	// Increment Residency Time
		// 	jobs[frameTable[i].first].total_resi_time++;
		// }
	}
}
void printInfo(){
	printf("The machine size is %lld.\n", M);
	printf("The page size is %lld.\n", P);
	printf("The process size is %lld.\n", S);
	printf("The job mix number is %lld.\n", J);
	printf("The number of references per process is %lld.\n", N);
	cout << "The replacement algorithm is " << R << endl;
	cout << endl;

	int total_fault = 0;
	int total_resi_total = 0;
	int total_evic = 0;
	for(int i = 1; i < jobs.size(); i++){
		Process& now = jobs[i];
		total_fault += now.num_of_fault;
		total_evic += now.num_of_evic;
		total_resi_total += now.total_resi_time;
		printf("Process %d has %d faults. ", now.idx, now.num_of_fault);
		if(now.num_of_evic==0){
			printf("With no evictions, the average residence is undefined.\n");
		} else {
			printf("%lf average residency.\n", 1.0*now.total_resi_time/now.num_of_evic);
		}
	}
	if(total_evic == 0){
		printf("The total number of faults is %d. There's no eviction\n", total_fault);
	} else
		printf("The total number of faults is %d and the overall average residency is %lf.\n", total_fault, 1.0*total_resi_total/total_evic);
}
int main(){
	cin >> M >> P >> S >> J >> N >> R;
	numFrames = M/P;
	frameTable.assign(numFrames,make_pair(-1,-1));
	freopen("random-numbers.txt", "r", stdin);
	ll tmp;
	while(scanf("%lld", &tmp) != EOF){
		random_number.push(tmp);
	}
	fclose(stdin);
	process();
	printInfo();
	return 0;
}