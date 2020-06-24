#include "Head.h"

//Normalize word
vector<string> normalize(string s, bool& isFinal) {
	int pos = s.length();
	int op = myOpSize;

	//Check final word of statement
	if (s.find(';') != std::string::npos) {
		isFinal = true;
		s.pop_back();
	}

	//Check if word contains ( or )
	if (s.find("(") != string::npos) {
		pos = s.find("(");

		//split
		if (pos > 0) {
			s.insert(pos, " ");
			++pos;
		}
		if (pos < s.length()-1) {
			s.insert(pos + 1, " ");
		}
	}
	if (s.find(")") != string::npos) {
		pos = s.find(")");

		//split
		if (pos > 0) {
			s.insert(pos, " ");
			++pos;
		}
		if (pos < s.length() - 1) {
			s.insert(pos + 1, " ");
		}
	}

	//Check if word contains operation
	for (int i = 0; i < myOpSize; ++i) {
		if (s.find(myOp[i]) != string::npos) {
			pos = s.find(myOp[i]);
			op = i;
		}
	}

	//Found operator in word
	if (op < myOpSize) {
		//split
		if (pos > 0) {
			s.insert(pos, " ");
			++pos;
		}
		if (pos < s.length() - 1) {
			s.insert(pos + myOp[op].length(), " ");
		}
	}

	std::istringstream buf(s);
	std::istream_iterator<std::string> beg(buf), end;

	vector<std::string> result(beg, end); 

	return result;
}

int checkType(string s, bool &stringMode) {
	//Check keyword
	for (int i = 0; i < myKwSize; ++i) {
		if (s == myKeyword[i]) {
			return 1;
		}
	}

	//Check integer
	if (regex_match(s, integer)) {
		return 3;
	}
	//Check float
	if (regex_match(s, floatRegex)) {
		return 4;
	}

	//Check operator
	for (int i = 0; i < myOpSize; ++i) {
		if (s == myOp[i]) {
			return 5;
		}
	}

	//Check identifier
	if (regex_match(s, identifier)) {
		return 2;
	}

	//Check string
	if (regex_match(s, startString) | regex_match(s, endString)) {
		stringMode = true;
		if (regex_match(s, endString))
			stringMode = false;
		return 8;
	}

	if (stringMode == true)
		return 8;

	//Check lpar & rpar
	if (s == "(")
		return 6;
	if (s == ")")
		return 7;

	//Check curly bracket
	if (s == "{" | s == "}")
		return 9;


	return 0;
}

void assignPriority(vector<word> &statement, int &level, vector<int> &type) {
	int typeID = 0;
	//vector <int> typeID;   //1-sum	2-mul  3-power
	//get unique operator in statement
	for (int i = 0; i < statement.size(); ++i) {
		if (statement[i].type == 5) {
			//cout << "Op " << statement[i].content << endl;
			if (statement[i].content == "+" | statement[i].content == "-") {
				typeID = 1;
				statement[i].priority = 1;
			}
			if (statement[i].content == "*" | statement[i].content == "/") {
				typeID = 2;
				statement[i].priority = 2;
			}
			if (statement[i].content == "^") {
				typeID = 3;
				statement[i].priority = 3;
			}
		}
		if (typeID > 0 && typeID < 4) {
			if (type.empty()) {
				type.push_back(typeID);
				//cout << "Push " << typeID << endl;
			}
			else {
				bool exist = false;
				for (int j = 0; j < type.size(); ++j) {
					if (typeID == type[j]) {
						exist = true;
					}
				}
				if (!exist) {
					type.push_back(typeID);
					//cout << "Push " << typeID << endl;
				}
			}
		}
	}
	//Sort type array in descending order --> ^ * +
	sort(type.begin(), type.end(), greater<int>());
	level = type.size() - 1;
	//cout << "level = " << level << endl;

}


//Convert grammar rule into code
//The factor -> look for an int or a float and return a number node
//The term -> look for a factor first, if the current token is the mul|div token, we will look for another factor

//Parser function
void myParser(vector <word> &statement, int start, int end, int &currentlevel, vector<int> type, vector<string> &result) {
	cout << start << " " << end << endl;
	cout << "level = " << currentlevel << endl;
	if (start == end || start == end - 1) {
		//push terminal here
		if (statement[start].type == 3) {
			//cout << statement[i].content << endl;
			cout << " Push int \n";
			result.push_back("int");
			statement[start].visited = true;
		}
		if (statement[start].type == 2) {
			cout << " Push variable \n";
			result.push_back("variable");
			statement[start].visited = true;
		}
	}
	for (int i = start; i < end; ++i) {
		cout << "\nCheck " << statement[i].content;
		if (statement[i].visited == false) {
			if (statement[i].content == "++") {
				result.push_back("increase");
				result.push_back("(id)");
			}
			if (statement[i].content == "--") {
				result.push_back("decrease");
				result.push_back("(id)");
			}
			if (statement[i].content == "=") {
				cout << " Push assign variable \n";
				result.push_back("assign(");
				result.push_back("variable,");
				statement[i].visited = true;
				statement[i - 1].visited = true;
				myParser(statement, i + 1, end, currentlevel, type, result);
			}
			if (statement[i].content == "+" && type[currentlevel] == statement[i].priority) {
				cout << " Push sum \n";
				result.push_back("sum(");
				statement[i].visited = true;
				--currentlevel;
				myParser(statement, start, i, currentlevel, type, result);
				result.push_back(","); 
				myParser(statement, i + 1, end, currentlevel, type, result);
				result.push_back(")");
				
			}
			if (statement[i].content == "*" && type[currentlevel] == statement[i].priority) {
				cout << " Push mul \n";
				result.push_back("mul(");
				statement[i].visited = true;
				--currentlevel;
				myParser(statement, start, i, currentlevel, type, result);
				result.push_back(",");
				myParser(statement, i + 1, end, currentlevel, type, result);
				result.push_back(")");
				--currentlevel;
			}
			if (statement[i].content == "^" && type[currentlevel] == statement[i].priority) {
				cout << " Push power \n";
				result.push_back("power(");
				statement[i].visited = true;
				--currentlevel;
				myParser(statement, start, i, currentlevel, type, result);
				result.push_back(",");
				myParser(statement, i + 1, end, currentlevel, type, result);
				result.push_back(")");
				--currentlevel;
			}
		}
	}
}