#ifndef MY_SHARED_DATA
#define MY_SHARED_DATA

#include <string>
#include <string.h>

// 共有メモリー上に配置する構造体
// C言語のデータとbitレベルで完全に互換性を持つPOD(Plain Old Data)型のみ利用できる
//	- ユーザー定義のコンストラクタ(引数なしのコンストラクタも)やコピーコンストラクタがないこと
//	- virtual関数やvirtual継承をしていないこと
//	- 参照メンバを持たないこと
//	- 継承していないこと
//	- C言語と互換性を持つプリミティブ型か配列のみ利用可能
// stringやmap,vector等は利用できないがメンバ関数の定義はできるので
// データ変換などをメンバ関数で行うようにするとstring <=> char[]などの相互変換ができる

struct SharedArea1
{
	struct Version
	{
		char system_version[64];
		char controller_version[64];
		std::string SystemVersion()
		{
			return std::string(system_version);
		}

		void SetSystemVersion(std::string _system_version)
		{
			::strcpy(system_version, _system_version.c_str());
		}
	} ver;

	struct System
	{
		int power_on;
		int time;
		bool serve_on;
	} sys;

	struct Angle
	{
		double ax[6]; //ax1,ax2,ax3,ax4,ax5,ax6
	} axis_angle;

	struct TCP
	{
		double tcp[6]; //x,y,z,rx,ry,rz
	} tool_tip;

	struct Logging
	{
		char start_time[15]; //20180104160301
		char stop_time[15];
	} logging;

};

#endif
