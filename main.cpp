#include <iostream>
#include <iomanip>
#include <fstream>
#include <string>
#include <cstring>
#include <sstream>
#include <windows.h>
#include <cstdio>
#include <io.h>
#include <ctime>
#include <cstdlib>
using namespace std;
struct Dataclimite {
    string STA_NO, YEAR, MONTH, DAY, DAT;
    double LAT, LOG, ALTI;
    double item[22][2];//第一维是数据，第二维是控制码，按顺序从蒸发量一直向后
};
double res[30][2];
const int INF = 0;
const int filename_lenth = 100;    
const string zero = "0";
const string title = "STA_NO	DAT	LAT	LOG	ALTI	S_EVAP	L_EVAP	S_EVAP_C	L_EVAP_C	A_S_TEMP	D_MIN_S_TEMP	D_MAX_S_TEMP	A_S_TEMP_C	D_MIN_S_TEMP_C	D_MAX_S_TEMP_C	PRE_20_8	PRE_8_20	C_PRE_20_20	PRE_20_8_C	PRE_8_20_C	C_PRE_20_20_C	A_S_PRES	D_MAX_S_PRES	D_MIN_S_PRES	A_S_PRES_C	D_MAX_S_PRES_C	D_MIN_S_PRES_C	A_R_HUMI	MIN_R_HUMI	A_R_HUMI_C	MIN_R_HUMI_C	S_DUR	S_DUR_C	A_TEMP	D_MAX_TEMP	D_MIN_TEMP	A_TEMP_C	D_MAX_TEMP_C	D_MIN_TEMP_C	A_W_SPE	MAX_W_SPE	MAX_W_SPE_D	M_MAX_W_SPE	M_MAX_W_SPE_D	A_W_SPE_D	MAX_W_SPE_D	MAX_W_SPE_D	M_MAX_W_SPE_D	M_MAX_W_SPE_D";

void station_file();
void ave_month_file();
void get_ave(Dataclimite &a);
void stan_unit(Dataclimite &a);

ostream &operator<<(ostream &out, const Dataclimite &data);
istream &operator>>(istream &in, Dataclimite &data);
int random(int n){
    return (long long)rand() * rand() % n; 
}

int main(){
	//station_file();
    srand((unsigned) time(0));
    ave_month_file();
	system("pause");
}

void get_sum(Dataclimite &info){
    for(int i = 0; i < 2; i++){
        if((0 == info.item[i][1] || 9 == info.item[i][1]) && info.item[i][0] != 32766 ) res[i][0] += info.item[i][0], res[i][1]++;
    }
    for(int i = 2; i < 5; i++){
        if((0 == info.item[i][1] || 9 == info.item[i][1]) && (info.item[i][0] != 32766) && ((int)info.item[i][0]/1000 != 31) && ((int)info.item[i][0]/1000 != 32) && ((int)info.item[i][0]/1000 != -10)  ) res[i][0] += info.item[i][0], res[i][1]++;
    }
    for(int i = 5; i < 8; i++){
            //cout << rand()%10 << endl;    
        if(32700 == info.item[i][0]) {
            info.item[i][0] = (double)random(1e4) / 1e4, info.item[i][1] = 0;
            if(i == 5) cout << info.item[5][0] << endl;
        }   
        else if((int)info.item[i][0]/1000 == 32 && info.item[i][0] != 32766) info.item[i][0] -= 32000, info.item[i][1] = 0;
        else if((int)info.item[i][0]/1000 == 31) info.item[i][0] -= 31000, info.item[i][1] = 0;
        else if((int)info.item[i][0]/1000 == 30) info.item[i][0] -= 30000, info.item[i][1] = 0;
        else if((int)info.item[i][0]/1000 == 9)  info.item[i][1] = 8;
        if((0 == info.item[i][1] || 9 == info.item[i][1])&& info.item[i][0] < 32766) res[i][0] += info.item[i][0], res[i][1]++;
    }

    for(int i = 8; i < 13; i++){
        if(0 == info.item[i][1]|| 9 == info.item[i][1]) res[i][0] += info.item[i][0], res[i][1]++;
    }
    if(info.item[13][0] > 240) info.item[13][1] = 8;
    if(0 == info.item[13][1]|| 9 == info.item[13][1]) res[13][0] += info.item[13][0], res[13][1]++;
    //cout << res[13][1] << endl;
    for(int i = 14; i < 19; i++){
        if(0 == info.item[i][1]|| 9 == info.item[i][1]) res[i][0] += info.item[i][0], res[i][1]++;
    }
                    
    if(0 == info.item[20][1]|| 9 == info.item[20][1]) res[20][0] += info.item[20][0], res[20][1]++;
    if((0 == info.item[19][1]|| 9 == info.item[19][1]) && info.item[19][0] >= 1 && info.item[19][0] <= 16) 
        res[19][0] += info.item[19][0], res[19][1]++;
    if((0 == info.item[21][1]|| 9 == info.item[21][1]) && info.item[21][0] >= 1 && info.item[21][0] <= 16) 
        res[21][0] += info.item[21][0], res[21][1]++;
}

void get_ave(Dataclimite &a){
	for(int i = 0; i < 22; i++){
		if(0 == res[i][1]){
			a.item[i][0] = INF;
			a.item[i][1] = 8;
		}
		else{
			double ave = res[i][0] / res[i][1] * 1.0;
			a.item[i][0] = ave;
			a.item[i][1] = 0;
		}
	}
    //cout << res[13][0] << " " << res[13][1] << endl;
	for(int i = 0; i < 22; i++){
		res[i][0] = res[i][1] = 0;
	}
	a.DAT = a.DAT.substr(0,6);
}

void ave_month_file(){
	std::string inPath = "E:\\climate\\3\\*";//遍历文件夹下的所有文件
	//用于查找的句柄
	long handle;
	struct _finddata_t fileinfo;
	//第一次查找
	handle = _findfirst(inPath.c_str(),&fileinfo);
	/*if(handle == -1)
	return -1;*/
	do{
			//读文件
		stringstream ss; 
		ss  << "E:\\climate\\3\\"<< fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy(q,linenum.c_str());
		ifstream fin(q, ios_base::in);
		string line;//用于存储每一行字符串  
        getline(fin, line);
		getline(fin, line);

		istringstream record(line);//字符串输入流对象
		Dataclimite ans;
        Dataclimite info;
		record >> ans;                         
        
		while(getline(fin, line)){
			istringstream rrecord(line);//字符串输入流对象
            rrecord >> info;
            
            //cout << info.DAT <<" "<< ans <<endl;
			if(info.DAT[5] == ans.DAT[5]) get_sum(info);
            else{
                get_sum(ans);
                get_ave(ans);
                stan_unit(ans);
				
				stringstream ss;
                ss << "E:\\climate\\ave_month\\"<<info.STA_NO << "aver_month.txt";
                string linenum = ss.str();
                char p[filename_lenth];
                strcpy(p,linenum.c_str());		
                ifstream file2(p);
                if(!file2){
                    file2.close();
                    ofstream file1(p,ios::app);
                    file1 << title << endl;
                }
                ofstream file1(p,ios::app);
				file1 << ans;
				file1.close();
                ans = info;
            }
        }        
        get_ave(info);

        stan_unit(info);

        //cout << info.STA_NO << endl;
        stringstream sss;
        sss << "E:\\climate\\ave_month\\"<<info.STA_NO << "aver_month.txt";
        linenum = sss.str();//改成sss会有bug，不改的话就在源文件里输出最后一个月？？？？？？？
        //找到原因啦，文件名字符串数组开太小了。。?
        cout  << sss.str() <<endl;
        char p[filename_lenth];
        strcpy(p,linenum.c_str());		
        ifstream file2(p);
        if(!file2){
            file2.close();
            ofstream file1(p,ios::app);
            file1 << title << endl;
        }
        ofstream file1(p,ios::app);
        file1 << info;
        file1.close();
        fin.close();
        ss.clear();
        ss.str("");
    } while (!_findnext(handle,&fileinfo));
    _findclose(handle);
}

void station_file(){
	clock_t start,finish;
	start = clock();
	std::string inPath = "E:\\climate\\1\\*";//遍历文件夹下的所有文件
	//用于查找的句柄
	long handle;
	struct _finddata_t fileinfo;
	//第一次查找
	handle = _findfirst(inPath.c_str(),&fileinfo);
	/*if(handle == -1)
	return -1;*/
	do{
			//读文件
		stringstream ss; 
		ss  << "E:\\climate\\1\\"<< fileinfo.name;
		cout << fileinfo.name;
		string linenum = ss.str();
		char q[filename_lenth];
		strcpy(q,linenum.c_str());		
		ifstream fin(q, ios_base::in);
		string line;//用于存储每一行字符串
		while(getline(fin, line)){
			istringstream record(line);//字符串输入流对象
			Dataclimite info;
			record >> info;

			//时间处理
			if(info.MONTH.size() == 1){
				info.MONTH = zero + info.MONTH; 
			}		
			if(info.DAY.size() == 1){
				info.DAY = zero + info.DAY; 
			}	
			info.DAT = info.YEAR + info.MONTH + info.DAY;
			
			//写文件
			stringstream ss;
			ss << "E:\\climate\\2\\"<<info.STA_NO << ".txt";
			string linenum = ss.str();
			char p[filename_lenth];
			strcpy(p,linenum.c_str());		
			ifstream file2(p);
			if(!file2){
				file2.close();
				ofstream file1(p,ios::app);
				file1 << title << endl;
			}
			ofstream file1(p,ios::app);
			file1 << info;
			file1.close();
            }
            fin.close();
            } while (!_findnext(handle,&fileinfo));
            _findclose(handle);
        finish = clock();
        cout << "time = " << double(finish-start)/CLOCKS_PER_SEC<<"s";
}

void stan_unit(Dataclimite &a){
    	for (int i = 0; i < 22; i++) {
			if(a.item[i][1] == 0) a.item[i][0] = a.item[i][0] / 10;
		}
        if(0 == a.item[11][1]) a.item[11][0] = a.item[11][0] / 10;
		if(0 == a.item[12][1]) a.item[12][0] = a.item[12][0] / 10;
		if(0 == a.item[19][1]) a.item[19][0] = a.item[19][0] * 10;
		if(0 == a.item[21][1]) a.item[21][0] = a.item[21][0] * 10;
}

istream &operator>>(istream &in, Dataclimite &data){
	in >> data.STA_NO >> data.DAT >> data.LAT >> data.LOG >> data.ALTI >> data.item[0][0] >> data.item[1][0] >> data.item[0][1] >> data.item[1][1] 
		>> data.item[2][0] >> data.item[3][0] >> data.item[4][0] >> data.item[2][1] >> data.item[3][1] >> data.item[4][1] 
		>> data.item[5][0] >> data.item[6][0] >> data.item[7][0] >> data.item[5][1] >> data.item[6][1] >> data.item[7][1]
		>> data.item[8][0] >> data.item[9][0] >> data.item[10][0] >> data.item[8][1] >> data.item[9][1] >> data.item[10][1] 
		>> data.item[11][0] >> data.item[12][0] >> data.item[11][1] >> data.item[12][1] >> data.item[13][0] >> data.item[13][1] 
		>> data.item[14][0] >> data.item[15][0] >> data.item[16][0] >> data.item[14][1] >> data.item[15][1] >> data.item[16][1] 
		>> data.item[17][0] >> data.item[18][0] >> data.item[19][0] >> data.item[20][0] >> data.item[21][0] >> data.item[17][1] >> data.item[18][1] >> data.item[19][1] >> data.item[20][1] >> data.item[21][1];
	return in;
}

ostream &operator<<(ostream &out, const Dataclimite &data){
	out << data.STA_NO << "\t" << data.DAT <<" "<< data.LAT << "\t" << data.LOG << "\t" << data.ALTI << "\t" << fixed << setprecision(3) << data.item[0][0] << "\t" << data.item[1][0] << "\t" << fixed << setprecision(0)<< data.item[0][1] << "\t" << data.item[1][1] 
		<< "\t" << fixed << setprecision(3) << data.item[2][0] << "\t\t" << data.item[3][0] << "\t\t" << data.item[4][0] << "\t\t" << fixed << setprecision(0)<< data.item[2][1] << "\t\t" << data.item[3][1] << "\t\t" << data.item[4][1] 
		<< "\t\t" << fixed << setprecision(3)<< data.item[5][0] << "\t" << data.item[6][0] << "\t" << data.item[7][0] << "\t\t" << fixed << setprecision(0)<< data.item[5][1] << "\t\t" << data.item[6][1] << "\t\t" << data.item[7][1]
		<< "\t\t" << fixed << setprecision(3)<< data.item[8][0] << "\t" << data.item[9][0] << "\t\t" << data.item[10][0] << "\t\t" << fixed << setprecision(0)<< data.item[8][1] << "\t\t" << data.item[9][1] << "\t\t" << data.item[10][1] 
		<< "\t\t" << fixed << setprecision(3)<< data.item[11][0] << "\t\t" << data.item[12][0] << "\t\t" << fixed << setprecision(0)<< data.item[11][1] << "\t\t" << data.item[12][1] << "\t\t" << fixed << setprecision(3)<<  data.item[13][0] << "\t" << fixed << setprecision(0)<<  data.item[13][1] 
		<< "\t" << fixed << setprecision(3)<< data. item[14][0] << "\t" << data.item[15][0] << "\t\t" << data.item[16][0] << "\t\t" << fixed << setprecision(0)<< data.item[14][1] << "\t\t" << data.item[15][1] << "\t\t" << data.item[16][1] 
		<< "\t\t" << fixed << setprecision(3)<< data.item[17][0] << "\t" << data.item[18][0] << "\t\t" << fixed << setprecision(0) << data.item[19][0] << "\t\t" << fixed << setprecision(3)<< data.item[20][0] << "\t\t" << fixed << setprecision(0)<< data.item[21][0] << "\t\t" << data.item[17][1] << "\t\t" << data.item[18][1] << "\t\t" << data.item[19][1] << "\t\t" << data.item[20][1] << "\t\t" << data.item[21][1] << endl;
	return out;
}
