# READ READ READ

>  插值和求月平均值的代码已经写好了，分别有一个函数接口
>
>  需要占用一下大家的CPU
>
>  注释写了一些，可能写得不太清楚，操作有问题问我哦
>
>  程序运行起来就可以干别的事情啦（可能要一个多小时？），不会占用太多时间



## 操作步骤

1. 下载代码库到本地

2. 打开**Visual** **Studio**,创建空项目

3. `Spline.h` 添加在**头文件**

4. `Spline.cpp` 和 `S_a_A main.cpp` 添加到**源文件**

5. 主函数里有两个函数

   ~~~c++
   int main()
   {
   	srand((unsigned)time(0));
   	//两个函数有先后顺序，先使用第一个函数，完成后使用第二个函数（使用一个注释掉另一个）
   
   	//1.查找错值，三次样条插值
   	Sta_Interpolation();
   	
   	//2.求月平均值
   	ave_month_file();
   	system("pause");
   }
   ~~~

   使用时有**先后顺序**，先使用第一个函数，完成后使用第二个函数（使用一个注释掉另一个）

6. 使用 `Sta_Interpolation()` 时

   - **输入路径**为站点文件夹（即`sta_no`）中的文件所在文件夹，可以自己随意创建文件夹，也可以按照这个路径创建文件夹，然后将站点文件放入。

     > std_no文件夹在我之前在群里发过的压缩包里，没有的话我再传一下

   - **输出文件路径**可自己定义，也可按照现有的创建，输出插值好的文件。

   - 注意，这里的输出文件夹可以是`ave_month_file()` 的输入文件夹

   ~~~c++
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
           //...中间代码省略了
           //...
           stringstream sss;
   		//输出文件路径，输出插值好的文件
   		sss << "E:\\climate\\spline\\" << info.STA_NO << ".txt";
   		linenum = sss.str();
   		char p[filename_lenth];
           //...
           //...
       }
       //...
   }
   
   ~~~

7. 使用`ave_month_file()` 时

   - **输入路径**为插值后站点文件夹（即上一个函数的输出文件夹`E:\\climate\\spline\\`），也可以自己随意创建文件夹，然后将插值后站点文件放入。
   - **输出文件路径**可自己定义，也可按照现有的创建，输出插值好的文件。

   ~~~c++
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
   		//...
           //...
   
           stringstream ss;
           ss << "E:\\climate\\ave_month\\" << info.STA_NO << "aver_month.txt";
           string linenum = ss.str();
           //...
           //...
       }
       //...
   }
   
   ~~~

## 具体分工

计算机专业同学有3个，分成三部分，`1~250`，`251~500`，`501~699`。

我完成第一部分，后两部分另外两位同学一人选一部分。

完成后别忘了把平均值文件压缩后发给我哦

### 本周计划完成，撒花 ヾ(≧▽≦*)o



> 插值法代码参考博客 https://www.cnblogs.com/yabin/p/6426849.html

