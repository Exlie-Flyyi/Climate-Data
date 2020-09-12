#include <cstdio>
#include <iostream>
#include "Spline.h"
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <io.h>
#include <map>
#include <vector>

using namespace std;
using namespace SplineSpace;
struct Dataclimite {
	string STA_NO, YEAR, MONTH, DAY, DAT;
	double LAT, LOG, ALTI;
	double item[22][2];//第一维是数据，第二维是控制码，按顺序从蒸发量一直向后
};
double res[30][2];
double u[22], v[22];//风向，v是南北分量，u是东西方向分量
const int INF = 0;
const int filename_lenth = 100;
const double PI = 3.1415926535;
const double WindAngle = 22.5*PI / 180;
const string zero = "0";
const string title = "STA_NO	DAT	LAT	LOG	ALTI	S_EVAP	L_EVAP	S_EVAP_C	L_EVAP_C	A_S_TEMP	D_MAX_S_TEMP	D_MIN_S_TEMP	A_S_TEMP_C	D_MAX_S_TEMP_C	D_MIN_S_TEMP_C	PRE_20_8	PRE_8_20	C_PRE_20_20	PRE_20_8_C	PRE_8_20_C	C_PRE_20_20_C	A_S_PRES	D_MAX_S_PRES	D_MIN_S_PRES	A_S_PRES_C	D_MAX_S_PRES_C	D_MIN_S_PRES_C	A_R_HUMI	MIN_R_HUMI	A_R_HUMI_C	MIN_R_HUMI_C	S_DUR	S_DUR_C	A_TEMP	D_MAX_TEMP	D_MIN_TEMP	A_TEMP_C	D_MAX_TEMP_C	D_MIN_TEMP_C	A_W_SPE	MAX_W_SPE	MAX_W_SPE_D	M_MAX_W_SPE	M_MAX_W_SPE_D	A_W_SPE_D	MAX_W_SPE_D	MAX_W_SPE_D	M_MAX_W_SPE_D	M_MAX_W_SPE_D";
ostream &operator<<(ostream &out, const Dataclimite &data);
istream &operator>>(istream &in, Dataclimite &data);
string attributes[22] = { "S_EVAP",	"L_EVAP", "A_S_TEMP", "D_MAX_S_TEMP", "D_MIN_S_TEMP", 	"PRE_20_8",	"PRE_8_20",	"C_PRE_20_20",	"A_S_PRES",	"D_MAX_S_PRES",	"D_MIN_S_PRES",	"A_R_HUMI",	"MIN_R_HUMI",	"S_DUR_C",	"A_TEMP",	"D_MAX_TEMP",	"D_MIN_TEMP",	"A_W_SPE",	"MAX_W_SPE",	"MAX_W_SPE_D",	"M_MAX_W_SPE",	"M_MAX_W_SPE_D" };

map<pair<string, int>, int> lack_num_month; //存下每一个月的缺失个数，第一个键值为<年月，气候项（用数组下标表示）>，第二个键值是缺值个数
vector<Dataclimite> list;  //存下每一个文件所有信息的容器


// ***插值主函数***
void Sta_Interpolation();  
//某一天（num）某一种气候值(ci)插值,data为年月字符串，用于映射lack_num_month中的一个键值
inline void Item_Interpolation(int num, string date, int ci); 
//标记未被标记的错误值
inline void Flag_error_value(Dataclimite &info);
//三次样条插值（单值），a[i]自变量，b[i]因变量，x是插值点
inline double Spline_one(double a[], double b[], double x);
//单位转置
inline void stan_unit(Dataclimite &a);
//随机数，用于降水量
inline int random(int n) {
	return (long long)rand() * rand() % n;
}


// ***求每一个站点的月平均值
void ave_month_file(); 
//每一个月的值求和
inline void get_sum(Dataclimite &info);
//求每个月平均值（除了风向，都是算数平均）
inline void get_ave(Dataclimite &a);
//风向计算（转化为0~360度方位角）
inline double wind_angle(double u, double v);



int main()
{
	srand((unsigned)time(0));
	//两个函数有先后顺序，先使用第一个函数，完成后使用第二个函数（使用一个注释掉另一个）

	//1.查找错值，三次样条插值
	Sta_Interpolation();
	
	//2.求月平均值
	ave_month_file();

	//验证文件中插值是否正确
	/*double a[] = { 1,2,3,7,8,9 };
	double b[] = { 63,80,77,81,90,82 };
	cout << Spline_one(a, b, 4);*/
	system("pause");
}

inline double Spline_one(double a[], double b[], double x) {
	try
	{
		SplineInterface* sp = new Spline(a, b, 6);	//使用接口，且使用默认边界条件
		double y;
		sp->SinglePointInterp(x, y);	//求x的插值结果y
		return y;
	}
	catch (SplineFailure sf)
	{
		cout << sf.GetMessage() << endl;
	}

}

inline void Flag_error_value(Dataclimite &info) {
	//数据规范化（有一些标记为合法，但实际不合法）
	//蒸发量
	for (int i = 0; i < 2; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] == 32766 || info.item[i][0] == 32700))
			info.item[i][1] = 1;
	}
	//地表气温
	for (int i = 2; i < 5; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] > 30000))
			info.item[i][1] = 1;
		//超出仪器最大值（上溢）
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == -10))
			info.item[i][0] += 10000;
		//小于仪器最小值（下溢）
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == 10))
			info.item[i][0] -= 10000;
	}
	//降水量
	for (int i = 5; i < 8; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] == 32766))
			info.item[i][1] = 1;
		//微量
		if (32700 == info.item[i][0]) {
			info.item[i][0] = (double)random(1e4) / 1e4, info.item[i][1] = 0;
		}
		//纯雾露霜
		else if ((int)info.item[i][0] / 1000 == 32 && info.item[i][0] != 32766)
			info.item[i][0] -= 32000, info.item[i][1] = 0;
		//雨雪总量
		else if ((int)info.item[i][0] / 1000 == 31)
			info.item[i][0] -= 31000, info.item[i][1] = 0;
		//雪量(仅包括雨夹雪，雪暴）
		else if ((int)info.item[i][0] / 1000 == 30)
			info.item[i][0] -= 30000, info.item[i][1] = 0;
		//错值
		else if ((int)info.item[i][0] / 1000 == 9)
			info.item[i][1] = 8;
	}
	//气压
	//气压极值取自定时值，在原值上加20000，故要减20000
	for (int i = 8; i < 11; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == 20))
			info.item[i][0] -= 20000;
	}
	//湿度
	//最小相对湿度
	for (int i = 11; i < 13; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 3))
			info.item[i][0] -= 300;
	}
	//日照时数
	for (int i = 13; i < 14; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] > 240))
			info.item[i][1] = 1;
	}
	//风速
	for (int i = 17; i < 19; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 10))
			info.item[i][0] -= 1000;
	}
	for (int i = 19; i < 22; i++) {
		//风速
		//达到上限
		if (20 == i) {
			if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 10))
				info.item[i][0] -= 1000;
		}
		//风向
		else if ((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] > 16) {
			info.item[i][1] = 1;
		}
	}
}

inline void Item_Interpolation(int num, string date, int ci) {
	vector<int> x, y; //存下用于插值的点
	double x0[10], y0[10];
	//大于1/3，标记无效，return
	if (lack_num_month[{date, ci}] > 10) {
		list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
		return;
	}
	//小于等于1/3
	else {
		//cnt是用于插值的点的个数，pos是第一个插值点的位置，用于求相对位置
		int k = -1, cnt = 0, pos = 0;
		//在缺值之前三个位置找要用的点
		for (int i = num - 3; i < num; i++) {
			if (i >= 0) { // 不小于所有日期最小值
				if ((0 == list[i].item[ci][1] || 9 == list[i].item[ci][1]) && list[i].item[ci][0] < 32766) { //插值点需要满足是合法数据
					if (!x.size()) {//没有插值点
						x.push_back(1);
						pos = i;//记录下第一个插值点位置
						y.push_back(list[i].item[ci][0]);
					}
					else {
						x.push_back(i - pos + 1);
						y.push_back(list[i].item[ci][0]);
					}
					cnt++;
				}
			}
		}
		if (cnt >= 1) { //缺值点之前满足条件的点大于1，可以继续
			//在缺值之后六个位置找要用的点
			for (int i = num + 1; i <= num + 6; i++) {
				if (i < list.size() && (0 == list[i].item[ci][1] || 9 == list[i].item[ci][1]) && list[i].item[ci][0] < 32766) {
					x.push_back(i - pos + 1);
					y.push_back(list[i].item[ci][0]);
					cnt++;
				if (cnt == 6) break; //满足六个点，跳出循环，准备执行插值
				}
			}
		}
		//前面不够1，故不用前后几天插值，准备用前后几年同一天计算
		else {
			pos = 0;
			x.clear(); y.clear(); cnt = 0; //清空
			//计算年份
			int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
			if (year % 4) year = 365;
			else year = 366;//闰年
			//在缺值之前三个位置（三年）找要用的点
			for (int i = num - 3 * year; i < num; i += year) {
				if (i >= 0) {
					if ((0 == list[i].item[ci][1] || 9 == list[i].item[ci][1]) && list[i].item[ci][0] < 32766) {
						if (!x.size()) {
							x.push_back(1);
							pos = i;
							y.push_back(list[i].item[ci][0]);
						}
						else {
							x.push_back((i - pos) / year + 1);
							y.push_back(list[i].item[ci][0]);
						}
						cnt++;
					}
				}
			}
			//同理
			if (cnt >= 1) {
				for (int i = num + year; i <= num + 5 * year; i++) {
					if (i < list.size() && (0 == list[i].item[ci][1] || 9 == list[i].item[ci][1])  && list[i].item[ci][0] < 32766) {
						x.push_back((i - pos) / year + 1);
						y.push_back(list[i].item[ci][0]);
						cnt++;
					if (cnt == 6) break;
					}
				}
			}
		}
		//如果插值点满足6个
		if (cnt == 6) {
			//自变量插差值小于等于9，误差小，满足
			if (x[5] - x[0] <= 9 && num - pos + 1 > x[0] && num - pos + 1 < x[5]) {
				for (int i = 0; i < 6; i++) { //vector值放入数组种
					x0[i] = x[i];
					y0[i] = y[i];
				}
				list[num].item[ci][0] = Spline_one(x0, y0, num - pos + 1);
				list[num].item[ci][1] = 0;
			}
			//不满足，不进行插值，保持不合法数据
			else list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
		}
		//不满足，不进行插值，保持不合法数据
		else list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
	}
}

void Sta_Interpolation() {
	//输入路径为站点文件夹（即sta_no）中的文件所在文件夹
	std::string inPath = "E:\\climate\\3\\*";//遍历文件夹下的所有文件
	long handle;
	struct _finddata_t fileinfo;
	handle = _findfirst(inPath.c_str(), &fileinfo);
	do {
		//读文件
		list.clear();
		lack_num_month.clear();
		stringstream ss;
		//和上面的路径要一致
		ss << "E:\\climate\\3\\" << fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy_s(q, strlen(linenum.c_str())+1,linenum.c_str());
		ifstream fin(q, ios_base::in);
		string line = "0";//用于存储每一行字符串  
		Dataclimite info;
		getline(fin, line);
		while (getline(fin, line)) {
			istringstream record(line);//字符串输入流对象
			record >> info;
			Flag_error_value(info);
			list.push_back(info);
			// 求出每个月缺失值的天数
			for (int i = 0; i <= 21; i++) {
				if (!(0 == info.item[i][1] || 9 == info.item[i][1]) || info.item[i][0] == 32766)
					lack_num_month[{info.DAT.substr(0, 6), i}]++;
			}
		}
		stringstream sss;
		//输出文件路径，输出插值好的文件
		sss << "E:\\climate\\spline\\" << info.STA_NO << ".txt";
		linenum = sss.str();
		char p[filename_lenth];
		strcpy_s(p, strlen(linenum.c_str()) + 1, linenum.c_str());
		cout << p << endl;
		ifstream file2(p);
		if (!file2) {
			file2.close();
			ofstream file1(p, ios::app);
			file1 << title << endl;
		}
		ofstream file1(p, ios::app);
		//处理容器中的值（即一个文件里的值）
		for (int i = 0; i < list.size(); i++) {
			for (int j = 0; j <= 21; j++) {
				if (!(0 == list[i].item[j][1] || 9 == list[i].item[j][1]) || list[i].item[j][0] == 32766) {
					Item_Interpolation(i, list[i].DAT.substr(0, 6), j);
				}
			}
			stan_unit(list[i]); //单位转置
			file1 << list[i];
		}
		fin.close();
		file1.close();
	} while (!_findnext(handle, &fileinfo));
	_findclose(handle);
	//for(int i = 0; i <= 21;i++) cout << lack_num_month[{"201105", i}] <<endl;
}






inline void stan_unit(Dataclimite &a) {
	for (int i = 0; i < 22; i++) {
		if (a.item[i][1] == 0 || 9 == a.item[i][1])
			a.item[i][0] = a.item[i][0] / 10;
	}
	if (0 == a.item[11][1] || 9 == a.item[11][1])
		a.item[11][0] = a.item[11][0] / 10;
	if (0 == a.item[12][1] || 9 == a.item[12][1])
		a.item[12][0] = a.item[12][0] / 10;
	if (0 == a.item[19][1] || 9 == a.item[19][1])
		a.item[19][0] = a.item[19][0] * 10;
	if (0 == a.item[21][1] || 9 == a.item[21][1])
		a.item[21][0] = a.item[21][0] * 10;
}

double wind_angle(double u, double v) {
	double temp = atan(u / v) * 180 / PI;
	if (u > 0 && v > 0) {
		return temp;
	}
	else if (u > 0 && v < 0) {
		return 180 + temp;
	}
	else if (u < 0 && v < 0) {
		return 180 + temp;
	}
	else if (u < 0 && v > 0) {
		return 360 + temp;
	}
	else if (u == 0 && v > 0) {
		return 0;
	}
	else if (u == 0 && v < 0) {
		return 180;
	}
	else if (u > 0 && v == 0) {
		return 90;
	}
	else if (u < 0 && v == 0) {
		return 270;
	}
	else if (u == 0 && v == 0) {
		return 0;
	}
}

inline void get_sum(Dataclimite &info) {
	for (int i = 0; i < 2; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] != 32766)
			res[i][0] += info.item[i][0], res[i][1]++;
	}
	for (int i = 2; i < 5; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] != 32766) && ((int)info.item[i][0] / 1000 != 31.000) && ((int)info.item[i][0] / 1000 != 32.000) && ((int)info.item[i][0] / 1000 != -10.000))
			res[i][0] += info.item[i][0], res[i][1]++;
	}
	for (int i = 5; i < 8; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] < 32766)
			res[i][0] += info.item[i][0], res[i][1]++;
	}

	for (int i = 8; i < 13; i++) {
		if (0 == info.item[i][1] || 9 == info.item[i][1])
			res[i][0] += info.item[i][0], res[i][1]++;
	}
	if (0 == info.item[13][1] || 9 == info.item[13][1])
		res[13][0] += info.item[13][0], res[13][1]++;
	for (int i = 14; i < 19; i++) {
		if (0 == info.item[i][1] || 9 == info.item[i][1])
			res[i][0] += info.item[i][0], res[i][1]++;
	}
	for (int i = 19; i < 22; i++) {
		if (20 == i) {
			if (0 == info.item[i][1] || 9 == info.item[i][1])
				res[i][0] += info.item[i][0], res[i][1]++;
		}
		//处理风向
		else if ((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] >= 1 && info.item[i][0] <= 16) {
			u[i] += sin(info.item[i][0] * WindAngle);
			v[i] += cos(info.item[i][0] * WindAngle);
			res[i][1]++;
		}
	}
}

inline void get_ave(Dataclimite &a) {
	for (int i = 0; i < 22; i++) {
		if (20 >= res[i][1]) {
			a.item[i][0] = INF;
			a.item[i][1] = 1;
		}
		else {
			//计算风向平均值
			if (i == 19 || i == 21) {
				u[i] /= res[i][1] * 1.0;
				v[i] /= res[i][1] * 1.0;
				a.item[i][0] = wind_angle(u[i], v[i]);
				a.item[i][1] = 0;
				//cout << u[i] << " " << v[i] << " " << a.item[i][0] << endl;
				u[i] = 0, v[i] = 0;
			}
			else {
				double ave = res[i][0] / res[i][1] * 1.0;
				a.item[i][0] = ave;
				a.item[i][1] = 0;
			}
		}
	}
	for (int i = 0; i < 22; i++) {
		res[i][0] = res[i][1] = 0;
	}
	//取年月值
	a.DAT = a.DAT.substr(0, 6);
}


void ave_month_file() {
	//输入文件为查之后的文件，文件路径可以自己改
	std::string inPath = "E:\\climate\\spline\\*";//遍历文件夹下的所有文件
	//用于查找的句柄
	long handle;
	struct _finddata_t fileinfo;
	//第一次查找
	handle = _findfirst(inPath.c_str(), &fileinfo);
	/*if(handle == -1)
	return -1;*/
	do {
		//读文件
		stringstream ss;
		//文件路径同上
		ss << "E:\\climate\\spline\\" << fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy_s(q, strlen(linenum.c_str())+1,linenum.c_str());
		ifstream fin(q, ios_base::in);
		string line;//用于存储每一行字符串  
		getline(fin, line);
		getline(fin, line);

		istringstream record(line);//字符串输入流对象
		Dataclimite ans;
		Dataclimite info;
		record >> ans;

		while (getline(fin, line)) {
			istringstream rrecord(line);//字符串输入流对象
			rrecord >> info;

			//同月，值相加
			if (info.DAT[5] == ans.DAT[5]) get_sum(info);
			//不同月，求上个月的平均值
			else {
				get_sum(ans);
				get_ave(ans);

				stringstream ss;
				ss << "E:\\climate\\ave_month\\" << info.STA_NO << "aver_month.txt";
				string linenum = ss.str();
				char p[filename_lenth];
				strcpy_s(p, strlen(linenum.c_str()) + 1, linenum.c_str());
				ifstream file2(p);
				if (!file2) {
					file2.close();
					ofstream file1(p, ios::app);
					file1 << title << endl;
				}
				ofstream file1(p, ios::app);
				file1 << ans;
				file1.close();
				ans = info;
			}
		}
		//求最后一个月的平均值
		get_ave(info);

		stringstream sss;
		//月平均值输出文件路径
		sss << "E:\\climate\\ave_month\\" << info.STA_NO << "aver_month.txt";
		linenum = sss.str();//改成sss会有bug，不改的话就在源文件里输出最后一个月？？？？？？？
		//找到原因啦，文件名字符串数组开太小了。。?
		char p[filename_lenth];
		strcpy_s(p, strlen(linenum.c_str()) + 1, linenum.c_str());
		ifstream file2(p);
		if (!file2) {
			file2.close();
			ofstream file1(p, ios::app);
			file1 << title << endl;
		}
		ofstream file1(p, ios::app);
		file1 << info;
		file1.close();
		fin.close();
		ss.clear();
		ss.str("");
	} while (!_findnext(handle, &fileinfo));
	_findclose(handle);
}


istream &operator>>(istream &in, Dataclimite &data) {
	in >> data.STA_NO >> data.DAT >> data.LAT >> data.LOG >> data.ALTI >> data.item[0][0] >> data.item[1][0] >> data.item[0][1] >> data.item[1][1]
		>> data.item[2][0] >> data.item[3][0] >> data.item[4][0] >> data.item[2][1] >> data.item[3][1] >> data.item[4][1]
		>> data.item[5][0] >> data.item[6][0] >> data.item[7][0] >> data.item[5][1] >> data.item[6][1] >> data.item[7][1]
		>> data.item[8][0] >> data.item[9][0] >> data.item[10][0] >> data.item[8][1] >> data.item[9][1] >> data.item[10][1]
		>> data.item[11][0] >> data.item[12][0] >> data.item[11][1] >> data.item[12][1] >> data.item[13][0] >> data.item[13][1]
		>> data.item[14][0] >> data.item[15][0] >> data.item[16][0] >> data.item[14][1] >> data.item[15][1] >> data.item[16][1]
		>> data.item[17][0] >> data.item[18][0] >> data.item[19][0] >> data.item[20][0] >> data.item[21][0] >> data.item[17][1] >> data.item[18][1] >> data.item[19][1] >> data.item[20][1] >> data.item[21][1];
	return in;
}

ostream &operator<<(ostream &out, const Dataclimite &data) {
	out << data.STA_NO << "\t" << data.DAT << " " << data.LAT << "\t" << data.LOG << "\t" << data.ALTI << "\t" << fixed << setprecision(3) << data.item[0][0] << "\t" << data.item[1][0] << "\t" << fixed << setprecision(0) << data.item[0][1] << "\t" << data.item[1][1]
		<< "\t" << fixed << setprecision(3) << data.item[2][0] << "\t\t" << data.item[3][0] << "\t\t" << data.item[4][0] << "\t\t" << fixed << setprecision(0) << data.item[2][1] << "\t\t" << data.item[3][1] << "\t\t" << data.item[4][1]
		<< "\t\t" << fixed << setprecision(3) << data.item[5][0] << "\t" << data.item[6][0] << "\t" << data.item[7][0] << "\t\t" << fixed << setprecision(0) << data.item[5][1] << "\t\t" << data.item[6][1] << "\t\t" << data.item[7][1]
		<< "\t\t" << fixed << setprecision(3) << data.item[8][0] << "\t" << data.item[9][0] << "\t\t" << data.item[10][0] << "\t\t" << fixed << setprecision(0) << data.item[8][1] << "\t\t" << data.item[9][1] << "\t\t" << data.item[10][1]
		<< "\t\t" << fixed << setprecision(3) << data.item[11][0] << "\t\t" << data.item[12][0] << "\t\t" << fixed << setprecision(0) << data.item[11][1] << "\t\t" << data.item[12][1] << "\t\t" << fixed << setprecision(3) << data.item[13][0] << "\t" << fixed << setprecision(0) << data.item[13][1]
		<< "\t" << fixed << setprecision(3) << data.item[14][0] << "\t" << data.item[15][0] << "\t\t" << data.item[16][0] << "\t\t" << fixed << setprecision(0) << data.item[14][1] << "\t\t" << data.item[15][1] << "\t\t" << data.item[16][1]
		<< "\t\t" << fixed << setprecision(3) << data.item[17][0] << "\t" << data.item[18][0] << "\t\t" << fixed << setprecision(3) << data.item[19][0] << "\t\t" << fixed << setprecision(3) << data.item[20][0] << "\t\t" << fixed << setprecision(3) << data.item[21][0]
		<< "\t\t" << fixed << setprecision(0) << data.item[17][1] << "\t\t" << data.item[18][1] << "\t\t" << data.item[19][1] << "\t\t" << data.item[20][1] << "\t\t" << data.item[21][1] << endl;
	return out;
}