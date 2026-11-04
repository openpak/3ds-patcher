#include "Backup.hpp"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <string>
#include <sys/stat.h>
static std::string R;
static void put(const std::string&p,const char*c){std::string f=R+p;for(size_t i=1;i<f.size();++i)if(f[i]=='/'){mkdir(f.substr(0,i).c_str(),0777);}FILE*x=fopen(f.c_str(),"wb");fputs(c,x);fclose(x);}
static std::string get(const std::string&p){FILE*x=fopen((R+p).c_str(),"rb");if(!x)return "<none>";char b[256]={0};fread(b,1,255,x);fclose(x);return b;}
int main(int,char**argv){R=argv[1];char e[200]="";
 put("/3ds/nimbus/update/0004013000003202.ips","NEWACT");put("/3ds/nimbus/update/nimbus.3gx","PLUG");
 put("/luma/sysmodules/0004013000003202.ips","USERACT");
 assert(Backup::stage(e,200)==2); assert(Backup::managed()==2); assert(Backup::stage(e,200)==0); assert(get("/3ds/nimbus/update/nimbus.3gx")=="<none>");
 assert(get("/luma/sysmodules/0004013000003202.ips")=="USERACT");
 assert(Backup::install(e,200)); assert(get("/luma/sysmodules/0004013000003202.ips")=="NEWACT");
 assert(get("/3ds/openpak/system/act.original")=="USERACT"); assert(get("/luma/plugins/nimbus.3gx")=="PLUG");
 assert(Backup::live()==2); assert(Backup::install(e,200));
 assert(get("/3ds/openpak/system/act.original")=="USERACT");
 assert(Backup::remove(e,200)); assert(get("/luma/sysmodules/0004013000003202.ips")=="USERACT");
 assert(get("/luma/plugins/nimbus.3gx")=="<none>"); assert(Backup::managed()==2); assert(Backup::live()==0);
 assert(Backup::install(e,200)); put("/luma/plugins/nimbus.3gx","EDITED");
 assert(!Backup::remove(e,200)); assert(strstr(e,"System files changed")); assert(get("/luma/plugins/nimbus.3gx")=="EDITED");
 // files carried inside the app
 put("/luma/plugins/nimbus.3gx","PLUG"); put("/appfs/openpak/nimbus.3gx","PLUG2"); assert(Backup::install(e,200)); assert(Backup::stage(e,200)==1); assert(Backup::live()==1);
 assert(Backup::install(e,200)); assert(get("/luma/plugins/nimbus.3gx")=="PLUG2");
 puts("ok");}
