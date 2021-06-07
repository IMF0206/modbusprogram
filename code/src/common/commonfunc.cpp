#include "commonfunc.h"
#include <iostream>
#include <sstream>

using namespace std;

bool isNum(std::string str)
{
    stringstream sin(str);  
	double d;  
	char c;  
	if(!(sin >> d))  
	{
		/*解释： 
            sin>>t表示把sin转换成double的变量（其实对于int和float型的都会接收），
			如果转换成功，则值为非0，如果转换不成功就返回为0 
        */
		return false;
	}
	if (sin >> c) 
	{
		/*解释：
		此部分用于检测错误输入中，数字加字符串的输入形式（例如：34.f），在上面的的部分（sin>>t）
		已经接收并转换了输入的数字部分，在stringstream中相应也会把那一部分给清除，
		此时接收的是.f这部分，所以条件成立，返回false 
          */ 
		return false;
	}  
	return true;
}