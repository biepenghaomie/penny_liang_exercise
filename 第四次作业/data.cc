#include<iostream>
#include<bits/stdc++.h>
using namespace std;
typedef long long ll;
const ll maxn=0x3f3f3f3f3f3f3f3fLL;

int main(){
	freopen("data.in","w",stdout);
	srand(time(0));
	for(ll i=0;i<maxn;i++){
		int tmp=(rand()%2==0?-1:1)*rand();
		cout<<tmp;
		if(i!=maxn-1){
			cout<<"\n";
		}
	}
}

