/*
[Unix, Daemons, TCP] Daemonizing an existing TCP server
PRODUCT:    C compilers*
OP/SYS:     Unix/Linux (with pthread support)
            OpenVMS (with pthread support)*
SOURCE:     Philippe Vouters
            Fontainebleau/France
Help improve this Web material : Send your comments to Philippe.Vouters@laposte.net

*
OVERVIEW:*

This code in the PROGRAM section is a direct application and a mixture of codes
found at http://www.enderunix.org/documents/eng/daemon.php and
../tima/Unix-OpenVMS-TCPIP-Possible_opened_sockets_leak_caused_by_Windows.html

*
*** CAUTION ****

This sample program has been tested using gcc V4.4.3 20100127 (Red Hat 4.4.3-4)
on Fedora 12, HP C V7.3-018 on OpenVMS IA64 V8.3-1H1 and IBM's XL C compiler
level 9.0.0.0 on AIX Version 5.3. However, we cannot guarantee its effectiveness
because of the possibility of error in transmitting or implementing it. It is
meant to be used as a template for writing your own program, and may require
modification for use on your system.

*_OpenVMS information:_*

When and if OpenVMS shall implement the Unix fork(), you ought to then be able
to run this source code below the Unix way on this operating system platform.

As of current, you may instead activate this daemon_auxserver code using:

      $ cc/reentrancy=multithread daemon_auxserver
      $ link/thread=(upcalls,multiple_kernel_threads) daemon_auxserver
      $ RUN /DETACH/ERROR=disk:[directory]daemon.log daemon_auxserver

Note: disk:[000000]directory.DIR should have a W:RW file protection.

To stop this detached process, enter, on OpenVMS IA64, the command:

     $ STOP/IMAGE/ID=xxxxxx

A non-portability issue between the Unix and the OpenVMS worlds is described as
a comment in routine serve_connection in daemon_auxserver.c. This specific
non-portability issue has been noticed using:

     $ analyze/image/inter sys$library:pthread$RTL.exe
     This is an OpenVMS IA64 (Elf format) shareable image file

     Image Identification Information, in section 8.

         Image name:                                 "PTHREAD$RTL"
         Global Symbol Table name:                   "PTHREAD$RTL"
         Image file identification:                  "V3.22-084"
         Image build identification:                 "XBPK-BL1-000000"
         Link identification:                        "Linker T02-28"
         Link Date/Time:                             30-AUG-2007 10:52:58.35
     Press RETURN to continue, or enter a period (.) for next file:
     .
     The analysis uncovered NO errors.

According to HP contacts, this accurate problem has been raised to HP OpenVMS
PTHREAD engineering. A closer conformance to PTHREAD standards may appear in
some future and the pthread_setcanceltype call for VMS systems within the
daemon_auxserver.c code ought to then no longer be necessary.

*_Windows information:_*

You may use the Windows technique described at
../tima/Windows-2000-background-task-Event-logger-C++-example.html
to create a background window. You may replace the pthread calls by pure Windows
process creation and synchronizing means calls.
A direct pthreaded application translation to the Windows world is developped
at: ../tima/All-OS-OpenSSL-client-threaded-server-example.html

*
PROGRAM NOTES:*

  Compile the program following way on your *nix operating system:

  A) For a Linux x86 32 bits executable:
     $ cc -m32 -pthread -o daemon_auxserver daemon_auxserver.c

  B) For a Linux x86_64 64 bits executable:
     $ cc -m64 -pthread -o daemon_auxserver daemon_auxserver.c

  On HP-UX B.11.31 IA64, with the HP-UX bundled cc compiler:

     $ cc -mt +DD64 -w -o daemon_auxserver daemon_auxserver.c

  On HP Tru64 (by default all executables are 64 bits)

     $ cc -pthread -o daemon_auxserver daemon_auxserver.c

  On AIX Version 5, using IBM's XL C compiler:
     (/usr/vac/bin must be in the PATH)

     $ xlc_r -q64 -o daemon_auxserver daemon_auxserver.c

Although not tested under this Unix platform, you would compile this way
to build a 64 bits executable:

  On Sun/Solaris 10, using Sun's C compiler:

     $ cc -mt -m64 -o daemon_auxserver -lsocket -lnsl daemon_auxserver.c

  Note for Sun/Solaris 10 cc command above : you might have to add -w to the
  command to suppress a possible warning message regarding a voluntarily
  unreachable return statement at the end of a function inside the
  daemon_auxserver.c code. In such case, the Sun's C compiler would behave
  much like the HP-UX cc bundled compiler and unlike all tested and mentioned
  other compilers.

To activate the program:

     $ ./daemon_auxserver

To use it as a TCP daemon server:

     $ telnet <host> 5020
     foo
     boo
     CTRL-C
     if not link closed, CTRL+ALT GR+]
     telnet> quit

To watch out what this daemon logs:

    $ tail -f /tmp/daemon.log

To terminate it:

    $ kill -TERM `cat /tmp/deamon.lock`

*_Unix/Linux Notes:_*

A typical daemon software is characterized by the following:

    $ ps -ef | grep daemon_auxserver
    UID        PID  PPID  C STIME TTY          TIME CMD
    philippe 30579     1  0 19:17 ?        00:00:00 ./daemon_auxserver

This means a parent pid equals 1 and no associated terminal.

As mentionned to the author and on Linux, an alternative to calling the
daemonize routine in the code below is to directly invoke the glibc daemon(3)
routine.

*
PROGRAM OUTPUT:*

Typical Linux Fedora 12 or AIX Version 5.3 session output:

    $ /usr/bin/telnet localhost 5020
    Trying 127.0.0.1...
    Connected to localhost.
    Escape character is '^]'.
    foo
    foo
    boo
    boo
    ^CConnection closed by foreign host.

    $ kill `cat /tmp/daemon.lock`
    $ cat /tmp/daemon.log
    Daemon - beginning of the program
    Daemon - Client address : 127.0.0.1
    Daemon - Client port : 50484
    received length ::> 5; data=foo0D0A
    received length ::> 5; data=boo0D0A
    received length ::> 5; data=FFF4FFFD06
    Closing connection
    terminate signal catched

PROGRAM:
*/

/************/
/* daemon_auxserver.c */
/*
 *        Copyright (C) 2010 by Philippe.Vouters@laposte.net
 *
 *    This program is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by the
 *    Free Software Foundation, either version 3 of the License, or
 *    any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program.  If not, see <http://www.gnu.org/licenses/>;.
 */
#include <ctype.h>
#include <fcntl.h>
#include <signal.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#ifdef __VMS
#include <unixio.h>
#else
#include <unistd.h>
#endif

#if defined (__osf__) || defined (__hpux)
#define socklen_t int
#endif

#if defined (__VMS)
typedef unsigned int socklen_t;
#endif

#ifndef PTHREAD_STACK_MIN
#define PTHREAD_STACK_MIN 65535
#endif

#ifndef FD_SET
#ifndef __DECC
struct fd_set_struct {u_char fds_bits[64/8];};
typedef struct fd_set_struct fd_set;
#endif
#define NFDBITS         sizeof(fd_set)/sizeof (u_char)
#define FD_SETSIZE      NFDBITS
#define FD_SET(n, p)    ((p)->fds_bits[(n)/NFDBITS] |= (1 << ((n) % NFDBITS)))
#define FD_CLR(n, p)    ((p)->fds_bits[(n)/NFDBITS] &= ~(1 << ((n) % NFDBITS)))
#define FD_ISSET(n, p)  ((p)->fds_bits[(n)/NFDBITS] & (1 << ((n) % NFDBITS)))
#define FD_ZERO(p)      memset((char *)(p), 0,sizeof(*(p)))
#endif

#define AUXSERVER_PORT 5020
#define CTRL_C 3

#define IAC 0xFF
// TELNET command introducer
#define DONT 0xFE
// NOT use option
#define DO 0xFD
// please, DO use option
#define WONT 0xFC
// I WILL NOT use option
#define WILL 0xFB
// I will use option
#define SB 0xFA
// TELNET subnegotiation introducer
#define GA 0xF9
// Go ahead
#define EL 0xF8
// Erase current line
#define EC 0xF7
// Erase current character
#define AYT 0xF6
// Are you there?
#define AO 0xF5
// Abort output
#define IP 0xF4
// Interrupt process
#define BREAK 0xF3
// Break
#define DM 0xF2
// Data mark
#define NOP 0xF1
// NO operation
#define SE 0xF0
// End of sub negotiation
#define EOR 0xEF
// End of record
#define SYNCH 0xEE
// Synchronize
#define TELOPT_BINARY 0
// DO NOT interpret data
#define TELOPT_ECHO 1
// Echo option
#define TELOPT_SGA 3
// Suppress Go Ahead option
#define TELOPT_STAT 5
// Status
#define TELOPT_TM 6
// Timing Mark option
#define TELOPT_LOGOUT 0x12
// Logout
#define TELOPT_TTYPE 0x18
// Terminal type option
#define TELOPT_WS 0x1F
// Negotiate about Window Size
#define TELOPT_SPEED 0x20
// Terminal speed option
#define TELOPT_FLOW  0x21
// Remote Flow Control
#define TELOPT_XDISP 0x23
// X display location
#define TELOPT_ENVIR 0x24
// Environment option
#define END_OF_RECORD 0x19
// end-of-record option

// TELOPT_ENVIR (environment) types - see RFC 1408
#define VAR 0
#define VALUE 1
#define ESC 2
#define USERVAR 3

// Sub-option qualifiers
//
#define TELQUAL_VAR 1
// Option for environment
#define TELQUAL_VALUE 0
// Option for environment
#define TELQUAL_IS 0
// Option is
#define TELQUAL_SEND 1
// Send option
#define EOFREC 239
// EOR marker

#define RUNNING_DIR     "/tmp"
#define LOCK_FILE       "daemon.lock"
#if defined (__unix__) || defined (__unix)
#define LOG_FILE        "daemon.log"
#elif defined (__VMS)
#define LOG_FILE       "SYS$ERROR:"
#endif

#define LINE_WIDTH 132
#define ROOT_UID (uid_t)0

typedef struct {
   pthread_attr_t *attr;
   int s;
} arg_t;

static unsigned char Abort[]={(unsigned char)IAC,(unsigned char)IP};

void log_message(char *filename,char *message);

void cleanup(int s)
{
  char message[LINE_WIDTH];

/*
 * If given, shutdown and close sock2.
 */
  if (shutdown(s,2) < 0){
      sprintf(message, "Daemon - Socket shutdown error: %s",strerror(errno));
      log_message(LOG_FILE,message);
  }

  if (close (s)){
      sprintf(message, "Daemon - Socket close error: %s",strerror(errno));
      log_message(LOG_FILE,message);
  }

} /* end cleanup*/

void translate_received(char *in, char *out, int len){
   register char *cp=out;
   for (;len;in++,len--){
        if (!isprint((int)*in)){
           sprintf(cp,"%02X",(unsigned char)*in);
           cp +=strlen(cp);
        }
        else{
           *cp++=*in;
        }
   }
   *cp=0;
}

void finish(void *ptr){
  arg_t *arg =(arg_t *)ptr;

  log_message(LOG_FILE,"Closing connection");
  cleanup (arg->s) ;
  free(arg->attr);
  free(arg);
}
/*
 * This is our per socket thread.
 */
void *serve_connection (void *ptr){
  fd_set readmask;
  register arg_t *arg =(arg_t *)ptr;
  register int r;
  char message[LINE_WIDTH];
  char received[LINE_WIDTH];

#if defined (__VMS)
  int old;
#endif

  pthread_cleanup_push(finish,arg);
  /*
   * By default the pthread cancelability is set to PTHREAD_CANCEL_ENABLE
   * as per Tru64 pthread_setcancelstate man. This is the case under most
   * tested *nix, including Linux.
   *
   * Unlike all tested *nix, this does not seem the case under OpenVMS IA64
   * V8.3-1H1 where the pthread_cancel(pthread_self) seems ineffective. Under
   * OpenVMS V8.3-1H1, the only way for the pthread_cancel(pthread_self) to be
   * effective is to set the setcanceltype to PTHREAD_CANCEL_ASYNCHRONOUS.
   */
#if defined (__VMS)
   if (pthread_setcanceltype (PTHREAD_CANCEL_ASYNCHRONOUS, &old) < 0){
       fprintf(stderr,"error setting PTHREAD_CANCEL_ENABLE cancelstate\n");
       exit(EXIT_FAILURE);
   }
#endif
  for (;;) {
      FD_ZERO(&readmask);
      FD_SET(arg->s,&readmask);

  /* Is the socket s readable ? If yes, the bit at position s in readmask   */
  /* will be set to one enabling us to receive data from the remote partner */

      switch(select (arg->s+1,&readmask,NULL,NULL,NULL)){
           case -1:
                sprintf(message, "Daemon - Socket select error: %s",strerror(errno));
                log_message(LOG_FILE,message);
                goto out;
           case 0 :
                log_message(LOG_FILE,"Daemon - No data received ??");
                continue;
           case 1 :
                if (FD_ISSET (arg->s,&readmask)){
                    /*
                     * Receive message from socket s
                     */

                    //strcpy(received,"Buenas");

                    if ((r = recv(arg->s, received ,sizeof (received), 0))< 0){
                         sprintf(message, "Daemon - Socket recv error: %s",strerror(errno));
                         log_message(LOG_FILE,message);
                         goto out;
                    }

                    char aux[9];

                    strcpy(aux,received);
                    time_t t;
                    struct tm *tm;
                    char fechayhora[100];
                    t=time(NULL);
                    tm=localtime(&t);

                    strftime(fechayhora, 100, "%H:%M", tm);

                    int hora=tm->tm_hour;
                    if (hora>0 && hora<12){
                    strcpy(received,"Buenos dias Sr.");

                    }
                    if(hora>=12 && hora<18){
                    strcpy(received,"Buenos tardes Sr.");

                    }
                    if(hora >=18 && hora <24){
                    strcpy(received,"Buenas noches Sr.");

                    }
                    strcat(received,aux);
                    strcat(received,fechayhora);

                    received[strlen(received)+1]='\0';
                    sprintf(message, " received length ::> %d", r);


                    if (r){
                        strcat(message,"; data=");
                       translate_received(received,message+strlen(message),r);
                    }

                    log_message(LOG_FILE,message);
                    if (r && received[0] != CTRL_C &&
                        memcmp(received,Abort,sizeof(Abort))){
                        r = send(arg->s, received, r, 0);
                        if (r == -1){
                            sprintf(message, "Daemon - Socket send  error: %s",strerror(errno));
                            log_message(LOG_FILE,message);
                            goto out;
                        }
                    }
                    if (!r)
                        goto out;
                    if (r && (received[0] == CTRL_C ||
                              !memcmp(received,Abort,sizeof(Abort))))
                        pthread_cancel(pthread_self());
                }
                break;
      }/* end switch */
  } /* end for(;;) */
out:;
   pthread_cleanup_pop(1);
   return(NULL);
}

int daemon_server(int argc,char **argv)
{
  int     s;                         /* listening socket           */
  int     on = 1;                    /* used for setsockopt        */
  struct  sockaddr_in s_name;        /* Address struct for s       */
  char    message[LINE_WIDTH];       /* message buffer.            */
  socklen_t namelength;
  arg_t *arg;
  pthread_t dynthread=(pthread_t)NULL;




  log_message(LOG_FILE,"Daemon - beginning of the program");
  if ((s = socket (AF_INET, SOCK_STREAM, 0)) == -1)
        {
        sprintf (message,"Daemon - socket error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
        }
#ifdef WITH_KEEPALIVE
  if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof (on)) < 0){
        sprintf (message,"Daemon - setsockopt error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
  }
#endif
  if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof (on)) < 0){
        sprintf (message,"Daemon - setsockopt error: %s",strerror(errno));
        log_message(LOG_FILE,message);
        exit (1) ;
  }
  s_name.sin_family = AF_INET ;
  s_name.sin_port = htons(AUXSERVER_PORT) ;
  s_name.sin_addr.s_addr = htonl(INADDR_ANY);
  if (bind (s,(struct sockaddr *)&s_name, sizeof (s_name))<0){
      sprintf (message,"Daemon - bind error: %s",strerror(errno));
      log_message(LOG_FILE,message);
      cleanup (s) ;
      exit(1);
  }

  if (listen (s, 5) < 0){
       sprintf (message,"Daemon - listen error: %s",strerror(errno));
       log_message(LOG_FILE,message);
       cleanup (s) ;
       exit(2);
  }
  for (;;){
      namelength = sizeof (s_name);
      arg = malloc(sizeof (arg_t));
      arg->s = accept (s,(struct sockaddr *)&s_name,&namelength) ;
      if (arg->s == -1){
          sprintf (message,"Daemon - accept error: %s",strerror(errno));
          log_message(LOG_FILE,message);
          cleanup (s);
          free(arg);
          exit(2);
      }

      //strcpy(message,"Buenas tardes");

      sprintf (message,"Daemon - Client address : %s",inet_ntoa(s_name.sin_addr)) ;
      log_message(LOG_FILE,message);
      sprintf (message,"Daemon - Client port : %d",ntohs(s_name.sin_port) ) ;
      log_message(LOG_FILE,message);
      /* Start the thread that will serve the connection */
      arg->attr = malloc(sizeof(pthread_attr_t));
      pthread_attr_init(arg->attr);
      pthread_attr_setstacksize(arg->attr,PTHREAD_STACK_MIN+BUFSIZ+10000);
      pthread_attr_setdetachstate(arg->attr,PTHREAD_CREATE_DETACHED);
      pthread_create(&dynthread,
                     arg->attr,
                     serve_connection,
                     arg);
   }
   /* Not reached */
   return 1;
} /* end daemon server */

void log_message(char *filename,char *message)
{
     FILE *logfile;

     logfile=fopen(filename,"a");
     if(!logfile) return;
     fprintf(logfile,"%s\n",message);
     fclose(logfile);
}

void signal_handler(int sig)
{
        switch(sig) {
        case SIGHUP:
                log_message(LOG_FILE,"hangup signal catched");
                break;
        case SIGTERM:
                log_message(LOG_FILE,"terminate signal catched");
                exit(0);
                break;
        }
}
void daemonize()
{
#if defined (__unix__) || defined (__unix)
   int i,lfp;
   char str[10];

   if(getppid()==1) return; /* if parent process == 1 then already a daemon */
   i=fork();
   if (i<0) exit(1); /* fork error */
   if (i>0) exit(0); /* parent exits */
   /*
    * child (daemon) continues
    *
    * For the setuid root to work, the user from a root account MUST
    * 1/ $ chown root ./daemon_auxserver
    * 2/ $ chmod u+s ./daemon_auxserver
    *
    * If the setuid root is indeed possible, one would activate
    * daemon_auxserver from a non-root account, WITHOUT the need for sudo'ing
    * on Linux and would read:
    * $ ./daemon_auxserver
    * $ ps -ef | grep daemon_auxserver
    * UID        PID  PPID  C STIME TTY          TIME CMD
    * root      1516     1  0 21:45 ?        00:00:00 ./daemon_auxserver
    */
   setuid(ROOT_UID);
   /* continue even if we coudn't setuid root */
   errno = 0;
   setsid(); /* obtain a new process group */
   for (i=getdtablesize();i>=0;--i) close(i); /* close all descriptors */
   i=open("/dev/null",O_RDWR); dup(i); dup(i); /* handle standart I/O */
   umask((mode_t)027); /* set newly created file permissions */
   chdir(RUNNING_DIR); /* change running directory */
   lfp=open(LOCK_FILE,O_RDWR|O_CREAT,0640);
   if (lfp<0) exit(1); /* can not open */
   if (lockf(lfp,F_TLOCK,0)<0) exit(0); /* can not lock */
   /* first instance continues */
   sprintf(str,"%d\n",getpid());
   write(lfp,str,strlen(str)); /* record pid to lockfile */
   signal(SIGCHLD,SIG_IGN); /* ignore child */
   signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
   signal(SIGTTOU,SIG_IGN);
   signal(SIGTTIN,SIG_IGN);
   signal(SIGHUP,signal_handler); /* catch hangup signal */
   signal(SIGTERM,signal_handler); /* catch kill signal */
#elif defined (__VMS)
   return;
#endif
}
int main(int argc, char **argv)
{
        daemonize();

        return daemon_server(argc,argv); /* run */
}

/*
REFERENCE(S):*

Refer to your gcc or cc or xlc_r man for the available options you may add
to your compile/link command.

To write portable code accross some *nix operating systems, a valuable Web site
providing on-line mans is at http://nixdoc.net/ Just one wish from the author,
it is hoped the information therein contained is kept up to date.

Sun/Solaris 10 mans are available at http://docs.sun.com/app/docs/coll/40.10
Sun/Solaris C compiler documentation on available options is Web available at:
http://docs.sun.com/source/820-3529/h_for_C_options.html
*/
