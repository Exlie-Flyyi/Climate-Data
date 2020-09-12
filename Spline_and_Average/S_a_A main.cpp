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
	double item[22][2];//��һά�����ݣ��ڶ�ά�ǿ����룬��˳���������һֱ���
};
double res[30][2];
double u[22], v[22];//����v���ϱ�������u�Ƕ����������
const int INF = 0;
const int filename_lenth = 100;
const double PI = 3.1415926535;
const double WindAngle = 22.5*PI / 180;
const string zero = "0";
const string title = "STA_NO	DAT	LAT	LOG	ALTI	S_EVAP	L_EVAP	S_EVAP_C	L_EVAP_C	A_S_TEMP	D_MAX_S_TEMP	D_MIN_S_TEMP	A_S_TEMP_C	D_MAX_S_TEMP_C	D_MIN_S_TEMP_C	PRE_20_8	PRE_8_20	C_PRE_20_20	PRE_20_8_C	PRE_8_20_C	C_PRE_20_20_C	A_S_PRES	D_MAX_S_PRES	D_MIN_S_PRES	A_S_PRES_C	D_MAX_S_PRES_C	D_MIN_S_PRES_C	A_R_HUMI	MIN_R_HUMI	A_R_HUMI_C	MIN_R_HUMI_C	S_DUR	S_DUR_C	A_TEMP	D_MAX_TEMP	D_MIN_TEMP	A_TEMP_C	D_MAX_TEMP_C	D_MIN_TEMP_C	A_W_SPE	MAX_W_SPE	MAX_W_SPE_D	M_MAX_W_SPE	M_MAX_W_SPE_D	A_W_SPE_D	MAX_W_SPE_D	MAX_W_SPE_D	M_MAX_W_SPE_D	M_MAX_W_SPE_D";
ostream &operator<<(ostream &out, const Dataclimite &data);
istream &operator>>(istream &in, Dataclimite &data);
string attributes[22] = { "S_EVAP",	"L_EVAP", "A_S_TEMP", "D_MAX_S_TEMP", "D_MIN_S_TEMP", 	"PRE_20_8",	"PRE_8_20",	"C_PRE_20_20",	"A_S_PRES",	"D_MAX_S_PRES",	"D_MIN_S_PRES",	"A_R_HUMI",	"MIN_R_HUMI",	"S_DUR_C",	"A_TEMP",	"D_MAX_TEMP",	"D_MIN_TEMP",	"A_W_SPE",	"MAX_W_SPE",	"MAX_W_SPE_D",	"M_MAX_W_SPE",	"M_MAX_W_SPE_D" };

map<pair<string, int>, int> lack_num_month; //����ÿһ���µ�ȱʧ��������һ����ֵΪ<���£�������������±��ʾ��>���ڶ�����ֵ��ȱֵ����
vector<Dataclimite> list;  //����ÿһ���ļ�������Ϣ������


// ***��ֵ������***
void Sta_Interpolation();  
//ĳһ�죨num��ĳһ������ֵ(ci)��ֵ,dataΪ�����ַ���������ӳ��lack_num_month�е�һ����ֵ
inline void Item_Interpolation(int num, string date, int ci); 
//���δ����ǵĴ���ֵ
inline void Flag_error_value(Dataclimite &info);
//����������ֵ����ֵ����a[i]�Ա�����b[i]�������x�ǲ�ֵ��
inline double Spline_one(double a[], double b[], double x);
//��λת��
inline void stan_unit(Dataclimite &a);
//����������ڽ�ˮ��
inline int random(int n) {
	return (long long)rand() * rand() % n;
}


// ***��ÿһ��վ�����ƽ��ֵ
void ave_month_file(); 
//ÿһ���µ�ֵ���
inline void get_sum(Dataclimite &info);
//��ÿ����ƽ��ֵ�����˷��򣬶�������ƽ����
inline void get_ave(Dataclimite &a);
//������㣨ת��Ϊ0~360�ȷ�λ�ǣ�
inline double wind_angle(double u, double v);



int main()
{
	srand((unsigned)time(0));
	//�����������Ⱥ�˳����ʹ�õ�һ����������ɺ�ʹ�õڶ���������ʹ��һ��ע�͵���һ����

	//1.���Ҵ�ֵ������������ֵ
	Sta_Interpolation();
	
	//2.����ƽ��ֵ
	ave_month_file();

	//��֤�ļ��в�ֵ�Ƿ���ȷ
	/*double a[] = { 1,2,3,7,8,9 };
	double b[] = { 63,80,77,81,90,82 };
	cout << Spline_one(a, b, 4);*/
	system("pause");
}

inline double Spline_one(double a[], double b[], double x) {
	try
	{
		SplineInterface* sp = new Spline(a, b, 6);	//ʹ�ýӿڣ���ʹ��Ĭ�ϱ߽�����
		double y;
		sp->SinglePointInterp(x, y);	//��x�Ĳ�ֵ���y
		return y;
	}
	catch (SplineFailure sf)
	{
		cout << sf.GetMessage() << endl;
	}

}

inline void Flag_error_value(Dataclimite &info) {
	//���ݹ淶������һЩ���Ϊ�Ϸ�����ʵ�ʲ��Ϸ���
	//������
	for (int i = 0; i < 2; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] == 32766 || info.item[i][0] == 32700))
			info.item[i][1] = 1;
	}
	//�ر�����
	for (int i = 2; i < 5; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] > 30000))
			info.item[i][1] = 1;
		//�����������ֵ�����磩
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == -10))
			info.item[i][0] += 10000;
		//С��������Сֵ�����磩
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == 10))
			info.item[i][0] -= 10000;
	}
	//��ˮ��
	for (int i = 5; i < 8; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] == 32766))
			info.item[i][1] = 1;
		//΢��
		if (32700 == info.item[i][0]) {
			info.item[i][0] = (double)random(1e4) / 1e4, info.item[i][1] = 0;
		}
		//����¶˪
		else if ((int)info.item[i][0] / 1000 == 32 && info.item[i][0] != 32766)
			info.item[i][0] -= 32000, info.item[i][1] = 0;
		//��ѩ����
		else if ((int)info.item[i][0] / 1000 == 31)
			info.item[i][0] -= 31000, info.item[i][1] = 0;
		//ѩ��(���������ѩ��ѩ����
		else if ((int)info.item[i][0] / 1000 == 30)
			info.item[i][0] -= 30000, info.item[i][1] = 0;
		//��ֵ
		else if ((int)info.item[i][0] / 1000 == 9)
			info.item[i][1] = 8;
	}
	//��ѹ
	//��ѹ��ֵȡ�Զ�ʱֵ����ԭֵ�ϼ�20000����Ҫ��20000
	for (int i = 8; i < 11; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 1000 == 20))
			info.item[i][0] -= 20000;
	}
	//ʪ��
	//��С���ʪ��
	for (int i = 11; i < 13; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 3))
			info.item[i][0] -= 300;
	}
	//����ʱ��
	for (int i = 13; i < 14; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] > 240))
			info.item[i][1] = 1;
	}
	//����
	for (int i = 17; i < 19; i++) {
		if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 10))
			info.item[i][0] -= 1000;
	}
	for (int i = 19; i < 22; i++) {
		//����
		//�ﵽ����
		if (20 == i) {
			if ((0 == info.item[i][1] || 9 == info.item[i][1]) && ((int)info.item[i][0] / 100 == 10))
				info.item[i][0] -= 1000;
		}
		//����
		else if ((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] > 16) {
			info.item[i][1] = 1;
		}
	}
}

inline void Item_Interpolation(int num, string date, int ci) {
	vector<int> x, y; //�������ڲ�ֵ�ĵ�
	double x0[10], y0[10];
	//����1/3�������Ч��return
	if (lack_num_month[{date, ci}] > 10) {
		list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
		return;
	}
	//С�ڵ���1/3
	else {
		//cnt�����ڲ�ֵ�ĵ�ĸ�����pos�ǵ�һ����ֵ���λ�ã����������λ��
		int k = -1, cnt = 0, pos = 0;
		//��ȱֵ֮ǰ����λ����Ҫ�õĵ�
		for (int i = num - 3; i < num; i++) {
			if (i >= 0) { // ��С������������Сֵ
				if ((0 == list[i].item[ci][1] || 9 == list[i].item[ci][1]) && list[i].item[ci][0] < 32766) { //��ֵ����Ҫ�����ǺϷ�����
					if (!x.size()) {//û�в�ֵ��
						x.push_back(1);
						pos = i;//��¼�µ�һ����ֵ��λ��
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
		if (cnt >= 1) { //ȱֵ��֮ǰ���������ĵ����1�����Լ���
			//��ȱֵ֮������λ����Ҫ�õĵ�
			for (int i = num + 1; i <= num + 6; i++) {
				if (i < list.size() && (0 == list[i].item[ci][1] || 9 == list[i].item[ci][1]) && list[i].item[ci][0] < 32766) {
					x.push_back(i - pos + 1);
					y.push_back(list[i].item[ci][0]);
					cnt++;
				if (cnt == 6) break; //���������㣬����ѭ����׼��ִ�в�ֵ
				}
			}
		}
		//ǰ�治��1���ʲ���ǰ�����ֵ��׼����ǰ����ͬһ�����
		else {
			pos = 0;
			x.clear(); y.clear(); cnt = 0; //���
			//�������
			int year = (date[0] - '0') * 1000 + (date[1] - '0') * 100 + (date[2] - '0') * 10 + (date[3] - '0');
			if (year % 4) year = 365;
			else year = 366;//����
			//��ȱֵ֮ǰ����λ�ã����꣩��Ҫ�õĵ�
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
			//ͬ��
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
		//�����ֵ������6��
		if (cnt == 6) {
			//�Ա������ֵС�ڵ���9�����С������
			if (x[5] - x[0] <= 9 && num - pos + 1 > x[0] && num - pos + 1 < x[5]) {
				for (int i = 0; i < 6; i++) { //vectorֵ����������
					x0[i] = x[i];
					y0[i] = y[i];
				}
				list[num].item[ci][0] = Spline_one(x0, y0, num - pos + 1);
				list[num].item[ci][1] = 0;
			}
			//�����㣬�����в�ֵ�����ֲ��Ϸ�����
			else list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
		}
		//�����㣬�����в�ֵ�����ֲ��Ϸ�����
		else list[num].item[ci][0] = 0, list[num].item[ci][1] = 1;
	}
}

void Sta_Interpolation() {
	//����·��Ϊվ���ļ��У���sta_no���е��ļ������ļ���
	std::string inPath = "E:\\climate\\3\\*";//�����ļ����µ������ļ�
	long handle;
	struct _finddata_t fileinfo;
	handle = _findfirst(inPath.c_str(), &fileinfo);
	do {
		//���ļ�
		list.clear();
		lack_num_month.clear();
		stringstream ss;
		//�������·��Ҫһ��
		ss << "E:\\climate\\3\\" << fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy_s(q, strlen(linenum.c_str())+1,linenum.c_str());
		ifstream fin(q, ios_base::in);
		string line = "0";//���ڴ洢ÿһ���ַ���  
		Dataclimite info;
		getline(fin, line);
		while (getline(fin, line)) {
			istringstream record(line);//�ַ�������������
			record >> info;
			Flag_error_value(info);
			list.push_back(info);
			// ���ÿ����ȱʧֵ������
			for (int i = 0; i <= 21; i++) {
				if (!(0 == info.item[i][1] || 9 == info.item[i][1]) || info.item[i][0] == 32766)
					lack_num_month[{info.DAT.substr(0, 6), i}]++;
			}
		}
		stringstream sss;
		//����ļ�·���������ֵ�õ��ļ�
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
		//���������е�ֵ����һ���ļ����ֵ��
		for (int i = 0; i < list.size(); i++) {
			for (int j = 0; j <= 21; j++) {
				if (!(0 == list[i].item[j][1] || 9 == list[i].item[j][1]) || list[i].item[j][0] == 32766) {
					Item_Interpolation(i, list[i].DAT.substr(0, 6), j);
				}
			}
			stan_unit(list[i]); //��λת��
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
		//�������
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
			//�������ƽ��ֵ
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
	//ȡ����ֵ
	a.DAT = a.DAT.substr(0, 6);
}


void ave_month_file() {
	//�����ļ�Ϊ��֮����ļ����ļ�·�������Լ���
	std::string inPath = "E:\\climate\\spline\\*";//�����ļ����µ������ļ�
	//���ڲ��ҵľ��
	long handle;
	struct _finddata_t fileinfo;
	//��һ�β���
	handle = _findfirst(inPath.c_str(), &fileinfo);
	/*if(handle == -1)
	return -1;*/
	do {
		//���ļ�
		stringstream ss;
		//�ļ�·��ͬ��
		ss << "E:\\climate\\spline\\" << fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy_s(q, strlen(linenum.c_str())+1,linenum.c_str());
		ifstream fin(q, ios_base::in);
		string line;//���ڴ洢ÿһ���ַ���  
		getline(fin, line);
		getline(fin, line);

		istringstream record(line);//�ַ�������������
		Dataclimite ans;
		Dataclimite info;
		record >> ans;

		while (getline(fin, line)) {
			istringstream rrecord(line);//�ַ�������������
			rrecord >> info;

			//ͬ�£�ֵ���
			if (info.DAT[5] == ans.DAT[5]) get_sum(info);
			//��ͬ�£����ϸ��µ�ƽ��ֵ
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
		//�����һ���µ�ƽ��ֵ
		get_ave(info);

		stringstream sss;
		//��ƽ��ֵ����ļ�·��
		sss << "E:\\climate\\ave_month\\" << info.STA_NO << "aver_month.txt";
		linenum = sss.str();//�ĳ�sss����bug�����ĵĻ�����Դ�ļ���������һ���£�������������
		//�ҵ�ԭ�������ļ����ַ������鿪̫С�ˡ���?
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