/* $Id$ */

#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <assert.h>
#include "m.h"
#include "er.h"
#include "dxl.h"

char *dxl_cmd=NULL;

int dxl_allows(char *typ,char *ps,char *s,int n) {
  int pid,status;

  if(!dxl_cmd) return 0;
  if((pid=fork())==0) {
    char **argv; int argc;
    char *p; int arg, i;

    argc=5; p=ps; arg=0;
    for(;;) {
      if(*p=='\0') {
        if(arg) {arg=0; ++argc;} else break;
      } else arg=1;
      ++p;
    }
    argv=(char**)m_alloc(argc,sizeof(char*));
    argv[--argc]=NULL;
    argv[--argc]=(char*)m_alloc(n+1,sizeof(char)); argv[argc][n]='\0'; strncpy(argv[argc],s,n);
    argv[0]=dxl_cmd; argv[1]="allows"; argv[2]=typ;
    i=3; if(i<argc) {
      for(;;) {
        argv[i++]=ps;
	if(i==argc) break;
        while(*(ps++));
      }
    }
    execv(dxl_cmd,argv);
    er_printf("dxl: cannot execute %s: %s\n",dxl_cmd,strerror(errno));
 } else if(pid>0) {
    wait(&status);
    return !WEXITSTATUS(status);
  }
  er_printf("dxl: %s\n",strerror(errno));
  return 0;
}

int dxl_equal(char *typ,char *val,char *s,int n) {
  int pid,status;

  if(!dxl_cmd) return 0;
  if((pid=fork())==0) {
    char *argv[]={dxl_cmd,"equal",typ,val,NULL,NULL};

    argv[4]=(char*)malloc(n+1); argv[4][n]='\0'; strncpy(argv[4],s,n);
    execvp(dxl_cmd,argv);
    er_printf("dxl: cannot execute %s\n",dxl_cmd,strerror(errno));
  } else if(pid>0) {
    wait(&status);
    return !WEXITSTATUS(status);
  }
  er_printf("dxl: %s\n",strerror(errno));
  return 0;
}
