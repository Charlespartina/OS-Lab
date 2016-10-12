#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <vector>
#include <map>
#include <string>
#include <cstring>
#include <set>
#include <sstream>
using namespace std;

// The limit of length of a symbol is 10.
namespace {
const int SYMBOL_LIMIT = 10;
const int MACHINE_SIZE = 600;
const char IMMEDIATE = '1';
const char ABSOLUTE = '2';
const char RELATIVE = '3';
const char EXTERNAL = '4';
const int TYPE_DIGIT_IDX = 4;
const int ADDR_LENGTH = 3;
struct Module{
	int base_;
	map<string, int> definition_;
	vector<string> useList_;
	vector<string> textList_;
	Module(
		int base,
		map<string, int> definition, 
		vector<string> useList, 
		vector<string> textList
	) : base_(base), definition_(definition), useList_(useList), textList_(textList) {}
};
map<string,int> notUsedGlobal;
}
string toString(int num){
	ostringstream ss;
	ss << num;
	return ss.str();
}
string addr2string(int addr){
	string addr_string = toString(addr);
	int padding = ADDR_LENGTH - addr_string.size();
	for(int i = 0; i < padding; i++){
		addr_string = "0" + addr_string;
	}
	return addr_string;
}

void printSymbolTable(map<string, int>& symbols){
	printf("\nSymbol Table\n");
	for(map<string,int>::iterator it = symbols.begin(); it != symbols.end(); it++){
		printf("%s=%d\n",(it->first).c_str(), it->second);
	}
	printf("\n");
}

string processWord(string& word, int size, int& base, vector<string>& useList, map<string, int>& symbols, set<string>& notUsedLocal) {
	string result = "";
	if(word[TYPE_DIGIT_IDX] == IMMEDIATE){
		result = word.substr(0,ADDR_LENGTH+1);
		return result;
	}
	if(word[TYPE_DIGIT_IDX] == ABSOLUTE){
		int addr = atoi(word.substr(1,ADDR_LENGTH).c_str());
		// Absolute address exceeds machine size
		if(addr >= MACHINE_SIZE){
			result = word.substr(0,1) + "000" + " Error: The absolute address exceeds the machine size. Will use 0.";
		} else {
			result = word.substr(0,ADDR_LENGTH+1);
		}
		return result;
	}
	if(word[TYPE_DIGIT_IDX] == RELATIVE){
		int addr = atoi(word.substr(1,ADDR_LENGTH).c_str());
		// Relative size exceeds module size
		if(addr >= size){
			result = word.substr(0,1) + "000" + " Error: The relative address exceeds the module size. Will use 0.";
		} else {
			addr += base;
			string addr_string = addr2string(addr);
			result = word.substr(0,1) + addr_string;
		}
		return result;
	} else {
		int idx = atoi(word.substr(1,ADDR_LENGTH).c_str());
		if(idx >= useList.size()){
			// Treat as immediate
			result = word.substr(0,ADDR_LENGTH+1) + " Error: The external address exceeds the length of use list. Will treate the address as immediate.";
		} else {
			string symbol = useList[idx];
			notUsedLocal.erase(symbol);
			// Will use 0 if not existed
			int external_address = symbols.count(symbol) != 0 ? symbols[symbol] : 0;
			string addr_string = addr2string(external_address);
			result = word.substr(0,1) + addr_string;
			if(symbols.count(symbol) == 0){
				result += " Error: The symbol " + symbol + " is not defined. Will use 0.";
			}
		}
		return result;
	}
}
int main(){
	vector<Module> allModules;
	map<string, int> symbols;
	int n; // n Modules
	cin >> n;
	int base = 0;

	// First Pass
	for(int i = 0; i < n; i++){
		int defSize = 0, useListSize = 0, textSize = 0;
		map<string, int> definition;
		vector<string> useList;
		vector<string> textList;
		cin >> defSize;
		for(int d = 0; d < defSize; d++){
			string name;
			int addr;
			cin >> name >> addr;
			if(name.size() > SYMBOL_LIMIT)
				printf(" Error: The symbol %s exceeds the length limit.\n", name.c_str());
			definition[name] = addr;
			if(symbols.count(name) == 0){
				symbols[name] = addr;
			} else {
				// Multiply Defined
				printf(" Error: The symbol %s is multiply defined. Will use the first definition.\n", name.c_str());
				definition.erase(name);
			}
			notUsedGlobal[name] = i;
		}
		cin >> useListSize;
		for(int u = 0; u < useListSize; u++){
			string name;
			cin >> name;
			useList.push_back(name);
		}
		cin >> textSize;
		for(int t = 0; t < textSize; t++){
			string text;
			cin >> text;
			textList.push_back(text);
		}
		for(map<string, int>::iterator it = definition.begin(); it != definition.end(); it++){
			if(it->second >= textSize){
				printf(" Error: The symbol %s has a relative address exceeding the size of the module. Will set the relative address to 0.\n", (it->first).c_str());
				symbols[it->first] = base;
			} else {
				symbols[it->first] = base + it->second;
			}
		}
		allModules.push_back(Module(base, definition, useList, textList));
		base += textSize;
	}
	printSymbolTable(symbols);

	printf("Memory Map\n");
	// Second Pass
	int cont = 0;
	for(int i = 0; i < n; i++){
		set<string> notUsedLocal;
		Module& now = allModules[i];
		vector<string>& useList = now.useList_;
		vector<string>& textList = now.textList_;
		for(int u = 0; u < useList.size(); u++){
			string symbol = useList[u];
			notUsedLocal.insert(symbol);
			notUsedGlobal.erase(symbol);
		}
		int& base = now.base_;
		for(int t = 0; t < textList.size(); t++){
			string text = textList[t];
			string result = processWord(text, textList.size(), base, useList, symbols, notUsedLocal);
			printf("%-3d: %s\n", cont++, result.c_str());
		}
		for(set<string>::iterator it = notUsedLocal.begin(); it != notUsedLocal.end(); it++){
			string symbol = *it;
			printf("Warning: The symbol %s is in the use list of module %d but isn't used\n", symbol.c_str(), i);
		}
	}
	for(map<string, int>::iterator it = notUsedGlobal.begin(); it != notUsedGlobal.end(); it++){
		string symbol = it->first;
		printf("Warning: The symbol %s is defined in module %d but isn't used\n", symbol.c_str(), it->second);
	}

}