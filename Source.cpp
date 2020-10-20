#include <iostream>
#include <cstdlib>
#include <string>
#include <fstream>
#include <vector>
using namespace std;
//使用atoi(const char *k) 以string轉換需要使用 k.c_str()函式

int Atoi(string s, int radix);  //n進位轉10進位
string intToA(int n, int radix);  //10進位轉n進位
string OPcode(string opcode);  //opcode
void openFile(vector <string>& text, vector <int>& capacity , string FileName);  //開檔並讀取內容(結束後關檔)
void calculateType(vector <string> &text, vector <int> &capacity, vector <int> &type);  //呼叫計算型別函式
void calculateLCV(vector <int> &type, vector <string> &text, vector < vector <string> >& LCV);  //呼叫計算LCV函式
void calculateLocation(vector < vector <string> >& LCV, vector <string>& loc);
vector <string> CalculateObjectCode(vector < vector <string> >& LCV, vector < vector <string> >& SymbolLoc);
vector < vector <string> > SymbolTable(vector < vector <string> >& LCV, vector <string>& loc);  //做出symbol Table
void W_SymbolTable(vector < vector <string> >& LCV, vector <string>& loc, string FileName);  //寫symbolTable的檔案
void W_SourceCode(vector <string>& loc, string FileName, vector < vector <string> >& LCV);  //輸出Source Code
void W_ObjectProgram(vector <string> &ObjProg, string FileName);
vector <string> ObjectProgram(vector < vector <string> >& LCV, vector <string> &loc, vector <string> &ObjectCode);
int Compare(vector < vector <string> >& SLO, string& target , int num);  //比較是否讀取過 並解回傳index

int main(void)
{
	vector <string> text;  //存取字串
	vector <int> capacity;  //紀錄一行有幾個字元
	vector <int> type;  //以決定label,code,value的型別

	//輸入檔案名稱
	string FileName;
	cout << "輸入檔案名稱 >";
	cin >> FileName;

	openFile(text, capacity,FileName);  //開檔並讀取內容(結束後關檔)
	calculateType(text, capacity, type);  //呼叫計算型別函式
	vector < vector <string> > LCV(type.size(), vector<string>(3));  //label/code/value二維陣列 X行3列
	calculateLCV(type, text, LCV);  //呼叫計算LCV函式
	vector <string> loc(LCV.size() + 1);  //存取location
	calculateLocation(LCV, loc);  //計算location
	vector < vector <string> > SymbolLoc = SymbolTable(LCV,loc);
	vector <string> ObjectCode = CalculateObjectCode(LCV, SymbolLoc);
	vector <string> ObjProg = ObjectProgram(LCV, loc, ObjectCode);

	//使用者輸入格式
	string* inp = new string[3];
	inp[2] = "";
	cout << "請輸入相關指令 >";
	cin >> inp[0] >> inp[1] >> inp[2];
	if (inp[0] == "SICASM" && inp[1] == FileName && (inp[2] == "" || inp[2] == "-s" || inp[2] == "-t" || inp[2] == "-a"))
	{
		if (inp[2] == "")
		{
			cout << "正在輸出..."+FileName+".obj";
			W_ObjectProgram(ObjProg, FileName);
		}

		else if (inp[2] == "-s")
		{
			cout << "正在輸出..." + FileName + ".obj"+" 和 "+FileName+".lst";
			W_ObjectProgram(ObjProg, FileName);
			W_SymbolTable(LCV, loc, FileName);
		}

		else if (inp[2] == "-t")
		{
			cout << "正在輸出..." + FileName + ".obj" + " 和 " + FileName + ".stb";
			W_ObjectProgram(ObjProg, FileName);
			W_SourceCode(loc, FileName, LCV);
		}

		else
		{
			cout << "正在輸出..." + FileName + ".obj,"+FileName + ".lst" + " 和 " + FileName + ".stb";
			W_ObjectProgram(ObjProg, FileName);
			W_SourceCode(loc, FileName, LCV);
			W_SymbolTable(LCV, loc, FileName);
		}
	}

	else
	{
		cout << "檔案輸入格式錯誤" << endl;
	}

	return 0;
}

void openFile(vector <string>& text, vector <int>& capacity, string FileName)
{
	FileName += ".asm";
	ifstream input(FileName);

	//擷取檔案內容
	if (!input) {
		cout << "無法讀取檔案\n";
	}
	else {
		do {
			string buffer;
			getline(input, buffer);
			if (buffer[0] == '.')  //註解不處理
				continue;
			text.push_back(buffer);  //存取字串
			capacity.push_back(buffer.length());  //紀錄一行有幾個字元
			//cout <<buffer<<": " << buffer.length() << "\n";  //測試輸出
		} while (!input.eof());
	}
	input.close();
}
void calculateType(vector <string>& text, vector <int>& capacity, vector <int>& type)
{
	//型別1= 一組字串 /型別2= 兩組字串 /型別3= 三組字串
	for (int i = 0; i < text.size() - 1; i++)  //幾行要run
	{
		int count = 0;
		char tag = 'f';  //預設tag值

		//做標記 (因為\t有兩個的會有兩個結果)
		if (text[i].at(0) == '\t')
			tag = 't';

		for (int j = 0; j < capacity.at(i); j++)  //跑n個字元次數
		{

			//cout << text[i].at(j);  //測試輸出
			if (text[i].at(j) == '\t')
				count++;
		}

		//cout << "\ncount: " << count << " tag: " << tag << endl;  //測試輸出

		//決定型別
		if (count == 1)
			type.push_back(1);
		else if (count == 2 && tag == 't')
			type.push_back(2);
		else if (count == 2 && tag == 'f')
			type.push_back(3);
		else
			type.push_back(3);

		//cout << type.back() << "\n";  //測試輸出
	}
}
void calculateLCV(vector <int> &type, vector <string> &text, vector < vector <string> > &LCV)
{
	string get1, get2;

	//分類型別內容  二維陣列[行數][哪種資料]
	for (int i = 0; i < type.size(); i++)
	{
		int counter = 0;
		get1 = "";
		
		switch (type.at(i))
		{
		case 1:  //只有code
			for (int j = 0; j <= text[i].size(); j++)  //讀取字串
			{
				if (text.at(i)[j] == '\0') {
					LCV[i][1] = get1; //code接收字串
					break;
				}
				else if (text.at(i)[j] == '\t') {}
				else {
					get2 = text.at(i)[j];
					get1 += get2;
				}
			}
			break;

		case 2:  //code+size
			for (int j = 0; j <= text[i].size(); j++)  //讀取字串
			{
				while (j == 0)
				{
					if (text.at(i)[j] == '\0') 
						break;
					else if (text.at(i)[j] == '\t') {}
					else {
						get2 = text.at(i)[j];
						get1 += get2;
					}
					break;
				}

				while (j >= 1)
				{
					if (text.at(i)[j] == '\0') {
						LCV[i][1+counter] = get1; //code接收字串
						break;
					}
					else if (text.at(i)[j] == '\t' && text.at(i)[j - 1] == '\t') {}  //連續兩個\t的時候跳過
					else if (text.at(i)[j] == '\t')
					{
						LCV[i][1+counter] = get1; //code接收字串
						counter++;
						get1 = "";
					}
					else {
						get2 = text.at(i)[j];
						get1 += get2;
					}
					break;
				}
			}
			break;

		case 3:  //label+code+size
			//int k = text[i].size();
			for (int j = 0; j <= text[i].size(); j++)  //讀取字串
			{
				while (j == 0)
				{
					if (text.at(i)[j] == '\0')
						break;
					else if (text.at(i)[j] == '\t') {}
					else {
						get2 = text.at(i)[j];
						get1 += get2;
					}
					break;
				}

				while (j >= 1)
				{
					if (text.at(i)[j] == '\0') {
						LCV[i][0+counter] = get1; //code接收字串
						break;
					}
					else if (text.at(i)[j] == '\t' && text.at(i)[j - 1] == '\t') {}  //連續兩個\t的時候跳過
					else if (text.at(i)[j] == '\t')
					{
						LCV[i][0+counter] = get1; //code接收字串
						counter++;
						get1 = "";
					}
					else {
						get2 = text.at(i)[j];
						get1 += get2;
					}
					break;
				}
			}
			break;
		}
	}

	//測試輸出
	/*
	for (int i = 0; i < type.size(); i++)
	{
		cout << "第" << i << "行: ";
		for (int j = 0; j < 3; j++)
		{
			cout << "j="<<j<<":"<<LCV[i][j] << " ";
		}
		cout << endl;
	}
	*/

}
void calculateLocation(vector < vector <string> >& LCV, vector <string>& loc)
{
	string one = "0";
	string two = "00";
	for (int i = 0; i < LCV.size()-2; i++)
	{
		if (LCV[i][1] == "START")
		{
			if (LCV[i][2].size() == 4)
			{
				loc[i] = two + LCV[i][2];
				loc[i + 1] = two + LCV[i][2];
			}
			else if (LCV[i][2].size() == 5)
			{
				loc[i] = one + LCV[i][2];
				loc[i + 1] = one + LCV[i][2];
			}
			else 
			{
				loc[i] = LCV[i][2];
				loc[i + 1] = LCV[i][2];
			}
			
		}
		else if (LCV[i][1] == "BYTE")
		{
			string buffer = LCV[i][2];

			if (buffer[0] == 'C')  //C的話看字元數
			{
				int counter = 0;
				int point = 1;
				do {
					point++;
					if (buffer[point] != '\'')
						counter++;
				} while (buffer[point + 1] != '\'');
				int convert = Atoi(loc[i], 16);  //location轉成10進位
				int total = counter + convert;  //10進位+counter
				//轉成16進位字串
				if (intToA(total, 16).length() == 4)
					loc[i + 1] = two + intToA(total, 16);
				else if (intToA(total, 16).length() == 5)
					loc[i + 1] = one + intToA(total, 16);
				else
					loc[i + 1] = intToA(total, 16);
			}
			else if (buffer[0] == 'X')  //X的話+1
			{
				int convert = Atoi(loc[i], 16);  //location轉成10進位
				int total = 1 + convert;  //10進位+1
				//轉成16進位字串
				if (intToA(total, 16).length() == 4)
					loc[i + 1] = two + intToA(total, 16);
				else if (intToA(total, 16).length() == 5)
					loc[i + 1] = one + intToA(total, 16);
				else
					loc[i + 1] = intToA(total, 16);
			}
		}
		else if (LCV[i][1] == "RESW")
		{
			int num = 3 * atoi(LCV[i][2].c_str());  //n個word (1個word:3byte)
			int convert = Atoi(loc[i], 16);  //location轉成10進位
			int total = num + convert;  //10進位相加
			//轉成16進位字串
			if (intToA(total, 16).length() == 4)
				loc[i + 1] = two + intToA(total, 16);
			else if (intToA(total, 16).length() == 5)
				loc[i + 1] = one + intToA(total, 16);
			else
				loc[i + 1] = intToA(total, 16);
		}
		else if (LCV[i][1] == "RESB")
		{
			int k = Atoi(loc[i], 16) + atoi(LCV[i][2].c_str()); //將location轉10進位相加
			loc[i + 1] = "0" + intToA(k, 16);  //相加結果轉16進位儲存
			if (intToA(k, 16).length() == 4)
				loc[i + 1] = two + intToA(k, 16);
			else if (intToA(k, 16).length() == 5)
				loc[i + 1] = one + intToA(k, 16);
			else
				loc[i + 1] = intToA(k, 16);
		}
		else if (LCV[i][1] == "END")
			break;
		else
		{
			int convert = Atoi(loc[i], 16);  //location轉成10進位
			int total = 3 + convert;  //10進位+3
			//轉成16進位字串
			if (intToA(total, 16).length() == 4)
				loc[i + 1] = two + intToA(total, 16);
			else if (intToA(total, 16).length() == 5)
				loc[i + 1] = one + intToA(total, 16);
			else
				loc[i + 1] = intToA(total, 16);
		}
	}
}
int Atoi(string s, int radix)   //n進位轉10進位 (s是給定的radix進位制字串)
{
	int ans = 0;
	for (int i = 0; i < s.size(); i++)
	{
		char t = s[i];
		if (t >= '0' && t <= '9') ans = ans * radix + t - '0';
		else ans = ans * radix + t - 'a' + 10;
	}
	return ans;
}
string intToA(int n, int radix)  //10進位轉n進位  (n是待轉數字，radix是指定的進位制)
{
	string ans = "";
	do {
		int t = n % radix;
		if (t >= 0 && t <= 9)    ans += t + '0';
		else ans += t - 10 + 'a';
		n /= radix;
	} while (n != 0);    //使用do{}while（）以防止輸入為0的情況
	reverse(ans.begin(), ans.end());
	return ans;
}
string OPcode(string opcode) {

	string str = "";

	if (opcode == "ADD")
		str = "18";
	else if (opcode == "AND")
		str = "40";
	else if (opcode == "COMP")
		str = "28";
	else if (opcode == "DIV")
		str = "24";
	else if (opcode == "J")
		str = "3C";
	else if (opcode == "JEQ")
		str = "30";
	else if (opcode == "JGT")
		str = "34";
	else if (opcode == "JLT")
		str = "38";
	else if (opcode == "JSUB")
		str = "48";
	else if (opcode == "LDA")
		str = "00";
	else if (opcode == "LDCH")
		str = "50";
	else if (opcode == "LDL")
		str = "08";
	else if (opcode == "LDX")
		str = "04";
	else if (opcode == "MUL")
		str = "20";
	else if (opcode == "OR")
		str = "44";
	else if (opcode == "RD")
		str = "D8";
	else if (opcode == "RSUB")
		str = "4C";
	else if (opcode == "STA")
		str = "0C";
	else if (opcode == "STCH")
		str = "54";
	else if (opcode == "STL")
		str = "14";
	else if (opcode == "STSW")
		str = "E8";
	else if (opcode == "STX")
		str = "10";
	else if (opcode == "SUB")
		str = "1C";
	else if (opcode == "TD")
		str = "E0";
	else if (opcode == "TIX")
		str = "2C";
	else if (opcode == "WD")
		str = "DC";

	return str;
}
void W_SymbolTable(vector < vector <string> >& LCV, vector <string>& loc, string FileName)
{
	FileName += ".stb";
	ofstream output(FileName);
	output << "Symbol	Value\n======	======\n";
	for (int i = 0; i < LCV.size(); i++)
	{
		if (LCV[i][0].empty())
			continue;
		else
			output << LCV[i][0] << "\t" << loc.at(i) << endl;
	}
	output.close();
}
void W_SourceCode(vector <string>& loc, string FileName, vector < vector <string> >& LCV)
{
	string p = FileName + ".lst";
	FileName += ".asm";
	ifstream input(FileName);
	ofstream output(p);
	vector <string> text;  //存取字串

	//擷取檔案內容
	if (!input) {
		cout << "無法讀取檔案\n";
	}
	else {
		do {
			string buffer;
			getline(input, buffer);
			text.push_back(buffer);  //存取字串
		} while (!input.eof());
	}

	int k = 0;  //計算location項數
	output << "Loc." << "\t" << "Source statement\n=====	==================================" << endl;
	for (int i = 0; i < text.size()-1; i++)
	{
		if (text.at(i)[0] == '.')
			output << "\t" << text.at(i) << endl;
		else
		{
			output << loc.at(k) << "\t" <<LCV[k][0]<<"\t" << LCV[k][1] << "\t" << LCV[k][2] << endl;
			k++;
		}
	}
	output.close();
	input.close();
}
vector < vector <string> > SymbolTable (vector < vector <string> >& LCV, vector <string>& loc)
{
	vector < vector <string> > s(LCV.size(), vector<string>(2));
	int count = 0;
	for (int i = 0; i < LCV.size(); i++)
	{
		if (LCV[i][0].empty())
			continue;
		else
		{
			s[count][0] = LCV[i][0];
			s[count][1] = loc.at(i);
			//cout << s[count][0] << "\t" << s[count][1] << endl;  //測試程式
			count++;
		}
	}
	s.resize(count);  //對vector重新配置記憶體
	return s;
}
vector <string> CalculateObjectCode(vector < vector <string> >& LCV, vector < vector <string> > &SymbolLoc)
{
	vector <string> obj(LCV.size() + 1);
	for (int i = 0; i < LCV.size(); i++)
	{
		if (LCV[i][1] == "START" or LCV[i][1] == "RESW" or LCV[i][1] == "RESB")
			continue;
		else if (LCV[i][1] == "WORD")
		{
			string buffer = LCV[i][2];
			buffer = intToA(Atoi(buffer, 10), 16);
			int Long = buffer.length();
			if (Long == 1)
				obj[i] = "00000" + buffer;
			else if (Long == 2)
				obj[i] = "0000" + buffer;
			else if (Long == 3)
				obj[i] = "000" + buffer;
			else if (Long == 4)
				obj[i] = "00" + buffer;
			else if (Long == 5)
				obj[i] = "0" + buffer;
			else
				obj[i] = buffer;
		}
		else if (LCV[i][1] == "BYTE")
		{
			string buffer = LCV[i][2];
			string get1, get2;

			int point = 1;

			if (buffer[0] == 'C')
			{
				string suffix;
				string fin;
				int counter = 0;
				do {
					point++;
					if (buffer[point] != '\'')
					{
						get2 = buffer[point];
						get1 += get2;
						counter++;
					}
				} while (buffer[point + 1] != '\'');

				for (int j = 0; j < counter; j++)
				{
					int convert = int(get1[j]);
					string temp = intToA(convert, 16);
					fin += temp;
				}
				obj[i] = fin;
			}
			else if (buffer[0] == 'X')
			{
				do {
					point++;
					if (buffer[point] != '\'')
					{
						get2 = buffer[point];
						get1 += get2;
					}
				} while (buffer[point + 1] != '\'');
				obj[i] = get1;
			}
		}
		else if (LCV[i][1] == "END")
			break;
		else
		{
			int k = 0;
			string prefix, suffix;
			string get1,get2;

			//如果有,則需要用此string以及下面迴圈
			string find;
			int tag = 0;
			int num = 0;
			for (int p = 0; p < LCV[i][2].size(); p++)
			{
				if (LCV[i][2].at(p) == ',')
				{
					tag = -1;
					num = p;
					break;
				}
				find += LCV[i][2].at(p);
			}

			prefix = OPcode(LCV[i][1]);
			while (true)
			{
				if (LCV[i][2].empty())
				{
					suffix = "000000";
					break;
				}
				else if (tag==-1)  //有,的情況 ex: buffer,X
				{
					while (true)
					{
						if (find == SymbolLoc[k][0])
						{
							suffix = SymbolLoc[k][1];
							suffix = intToA((32768 + Atoi(suffix, 16)), 16);  //32768是8000的10進位
							if (suffix.length() == 4)
								suffix = "00" + suffix;
							else if (LCV[i][2].size() == 5)
								suffix = "0" + suffix;
							break;
						}
						k++;
					}
					break;
				}
				else if (LCV[i][2] == SymbolLoc[k][0] )
				{
					suffix = SymbolLoc[k][1];
					break;
				}
				k++;
			}

			//去除多餘的0
			for (int j = 0; j < 4; j++)
			{
				get2 = suffix[2 + j];
				get1 += get2;
			}
			obj[i] = prefix + get1;
		}

	}

	/*  測試輸出
	for (int i = 0; i < ObjectCode.size()-2; i++)
	{
		cout << "i=" << i << ":\t" << ObjectCode[i] << endl;
	}
	*/

	return obj;
}
vector <string> ObjectProgram(vector < vector <string> >& LCV, vector <string>& loc, vector <string>& ObjectCode)
{
	vector <string> s(LCV.size() + 1);  //紀錄object program
	vector < vector <string> > SLO(LCV.size() + 1, vector<string>(3));  //已讀取的symbol/location/objectcode
	vector < vector <string> > Waiting(LCV.size() + 1, vector<string>(2));  //存取尚未讀取到的指令 以及該記憶體位置

	int counter = 0;  //計算ObjectProgram
	string buffer;  //放結尾
	string first;  //放開頭
	int maxbyte = 0;

	for (int i = 0; i < LCV.size(); i++)
	{
		//遇到之前為定義的symobl
		do {
			int p = Compare(Waiting, LCV[i][0], i);
			if (p >= 0 && !LCV[i][0].empty())
			{
				if (maxbyte != 0)  //把之前的輸出
				{
					//將byte轉字元串接在buffer後面
					string x =intToA(maxbyte, 16);
					if (x.length() == 1)
						x = "0" + x;
					buffer = x + buffer;

					s[counter] = first + buffer;

					//初始化
					buffer = "";
					first = "";
					counter++;
					maxbyte = 0;
				}

				buffer = intToA((Atoi(loc.at(p), 16) + 1), 16);  //記憶體位置+1
				if (buffer.length() == 4)
					buffer = "00" + buffer;
				else if (buffer.size() == 5)
					buffer = "0" + buffer;
				buffer = buffer + "02";
				//----------------------以上串接到byte-------------------------------

				string z;
				for (int m = 2; m < 6; m++)
					z += loc.at(i)[m];
				buffer += z;
				
				s[counter] = "T" + buffer;
				//初始化
				buffer = "";
				first = "";
				counter++;
				maxbyte = 0;

				//移除並添加到SLO字典中
				SLO[i][0] = LCV[i][0];
				SLO[i][1] = loc[i];
				Waiting[p][0].erase();
				continue;
			}
		} while (Compare(Waiting, LCV[i][0], i) != -1 && !LCV[i][0].empty());  //找到多個為定義的情況

		if (maxbyte == 0)  //第一筆資料地址
		{
			first = "T" + loc.at(i);
		}

		if (LCV[i][1] == "START")
		{
			SLO[i][0] = LCV[i][0];
			SLO[i][1] = loc[i];
			SLO[i][2] = ObjectCode[i];
			string last = loc[loc.size() - 2];  //最後一位的位置
			string fin = intToA((Atoi(last, 16) - Atoi(loc[i], 16)), 16);  //總共使用位置
			if (fin.length() == 4)
				fin = "00" + fin;
			else if (fin.size() == 5)
				fin = "0" + fin;
			buffer = "H" + LCV[i][0] + "\t" + loc[i] + fin;  //串接
			s[counter] = buffer;
			counter++;
			buffer = "";
		}

		else if (LCV[i][1] == "BYTE" or LCV[i][1] == "WORD")
		{
			maxbyte += 3;
			if (maxbyte >= 30)
			{
				//將byte轉字元串接在buffer後面
				string x = intToA(maxbyte, 16);
				if (x.length() == 1)
					x = "0" + x;
				buffer = x + buffer;

				s[counter] = first + buffer;
				first = "";
				buffer = "";
				counter++;
				maxbyte = 0;
				i--;  //讓讀取到的放到下一行
			}
			else
			{
				SLO[i][0] = LCV[i][0];
				SLO[i][1] = loc[i];
				SLO[i][2] = ObjectCode[i];
				buffer += ObjectCode[i];
			}
		}

		else if (LCV[i][1] == "RESW" or LCV[i][1] == "RESB")
		{
			SLO[i][0] = LCV[i][0];
			SLO[i][1] = loc[i];
			if (LCV[i + 1][1] != "RESW" && LCV[i + 1][1] != "RESB")
			{
				//將byte轉字元串接在buffer後面
				string x = intToA(maxbyte, 16);
				if (x.length() == 1)
					x = "0" + x;
				buffer = x + buffer;

				s[counter] = first + buffer;
				first = "";
				buffer = "";
				counter++;
				maxbyte = 0;
			}
		}

		else if(LCV[i][1] == "END")
		{
			if (maxbyte != 0)  //把之前的輸出
			{
				//將byte轉字元串接在buffer後面
				string x = intToA(maxbyte, 16);
				if (x.length() == 1)
					x = "0" + x;
				buffer = x + buffer;

				s[counter] = first + buffer;

				//初始化
				buffer = "";
				first = "";
				counter++;
				maxbyte = 0;
			}

			int k = Compare(SLO, LCV[i][2], i);

			if (k >= 0)
			{
				buffer = SLO[k][1];
			}
			s[counter] = "E" + buffer;
		}

		else
		{
			int k = Compare(SLO, LCV[i][2], i);  //找已讀取過的字典
			if (k >= 0)
			{
				maxbyte += 3;
				if (maxbyte >= 30)
				{
					//將byte轉字元串接在buffer後面
					string x = intToA(maxbyte, 16);
					if (x.length() == 1)
						x = "0" + x;
					buffer = x + buffer;

					s[counter] = first + buffer;
					first = "";
					buffer = "";
					counter++;
					maxbyte = 0;
					i--;  //讓讀取到的放到下一行
				}
				else
				{
					SLO[i][0] = LCV[i][0];
					SLO[i][1] = loc[i];
					SLO[i][2] = ObjectCode[i];
					buffer += ObjectCode[i];
				}
			}
			else
			{
				maxbyte += 3;
				SLO[i][0] = LCV[i][0];
				SLO[i][1] = loc[i];
				SLO[i][2] = ObjectCode[i];
				Waiting[i][0] = LCV[i][2];  //value存取
				Waiting[i][1] = loc.at(i);  //locataion存取

				string z;
				for (int m = 0; m < 2; m++)  //讀取ObjectCode前兩碼
					z += ObjectCode.at(i)[m];
				z = z + "0000";
				buffer += z;
			}
		}
	}
	s.resize(counter + 1);
	return s;
}

int Compare(vector < vector <string> >& SLO, string &target,int num)  //比較目標是否被讀取過 if有回傳index else回傳-1
{
	int index = -1;
	
	for (int i = 0; i < num; i++)
	{
		if (SLO[i][0] == target)
		{
			index = i;
			break;
		}
	}

	return index;
}
void W_ObjectProgram(vector <string> &ObjProg, string FileName)
{
	FileName += ".obj";
	ofstream output(FileName);

	for (int i = 0; i < ObjProg.size(); i++)
	{
		output << ObjProg.at(i) << endl;
	}

	output.close();
}