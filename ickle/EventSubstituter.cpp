/*
 * EventSubstituter
 * Copyright (C) 2001 Barnaby Gray <barnaby@beedesign.co.uk>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or (at
 * your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 */

#include "EventSubstituter.h"

#include <libicq2000/socket.h>  // for IPtoString()

#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>

#if HAVE_SYS_WAIT_H
# include <sys/wait.h>
#endif
#ifndef WEXITSTATUS
# define WEXITSTATUS(stat_val) ((unsigned)(stat_val) >> 8)
#endif
#ifndef WIFEXITED
# define WIFEXITED(stat_val) (((stat_val) & 255) == 0)
#endif

#include <signal.h>
#include <string.h>

using std::ostringstream;

using ICQ2000::ContactRef;
using ICQ2000::Status;

EventSubstituter::EventSubstituter(MessageQueue& mq, const ContactRef& c)
  : m_message_queue(mq)
{
  co = c;
  line_start = true;
  got_special = 0;
  event_time = time(NULL);
  escape_shell = false;
  repeated = false;
}

static const char safe_chars[]
    = "\t\n !#%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[]^_"
      "abcdefghijklmnopqrstuvwxyz{|}~";
static const char escape_chars[] = "$\\`\"";

// Cats the string onto the stream, escaping shell metachars if (escape_shell)
void EventSubstituter::sanecat(const string &s) {
  if (!escape_shell) {
    static_cast<ostringstream&>(*this) << s.c_str();  
    return;
  }

  // Surround the substitution with double quotes. Allow through a certain
  // set of characters, escape another set with backslashes, and chop the rest.
  static_cast<ostringstream&>(*this) << '"';
  for (const char *c = s.c_str(); *c; c++) {
    if (strchr(safe_chars, *c) != NULL) {
      static_cast<ostringstream&>(*this) << (*c);
    }
    else if (strchr(escape_chars, *c) != NULL) {
      static_cast<ostringstream&>(*this) << '\\';
      static_cast<ostringstream&>(*this) << (*c);
    }
  }
  static_cast<ostringstream&>(*this) << '"';
}

void EventSubstituter::execute() 
{
  // we have the entire pipe command in `cmd' now.
  // Make % substitutions.
  EventSubstituter cmdsub(m_message_queue, co);
  cmdsub.set_event_time(event_time);
  cmdsub.set_escape_shell(true);
  cmdsub << cmd.c_str();
  cmd.erase();
  
  PipeExec pp;
  pp.Open(cmdsub.str().c_str());
  pp.CloseInput();
  
  char buf[1024];
  pp.Read(buf, sizeof(buf));
  pp.Close();
  
  sanecat(buf);
}

EventSubstituter& EventSubstituter::operator<<(char c) {
  if (c == '\n') {
    line_start = true;

    if (got_special == '|') execute();
    got_special = 0;
    static_cast<ostringstream&>(*this) << c;
    // bug in g++ 3.xx - operator<<(char c) method doesn't exist
    return (*this);  // C++ is silly.
  }

  else if (got_special == '%') {
    // process % subst
    got_special = 0;
    char timebuf[100];  // for %t, %T

    switch (c) {
      // We call the ostringstream << methods and not EventSubstituter's to
      // ensure substitutions aren't processed recursively.
      case 'i':
	sanecat(IPtoString(co->getExtIP()));
	break;
      case 'p':
        static_cast<ostringstream&>(*this) << co->getExtPort();
	break;
      case 'e':
        sanecat(co->getEmail());
	break;
      case 'n':
	sanecat(co->getFirstName() + " " + co->getLastName());
	break;
      case 'f':
        sanecat(co->getFirstName());
	break;
      case 'l':
        sanecat(co->getLastName());
	break;
      case 'a':
        sanecat(co->getAlias());
	break;
      case 'u':
        sanecat(co->getStringUIN());
	break;
      // case 'w': would be the web address
      // case 'h': would be home phone number
      case 'c':
        sanecat(co->getMobileNo());
	break;
      case 's':
      case 'S':
        sanecat(co->getStatusStr());
	break;
      case 't':
        strftime(timebuf, 100, "%b %d %r", localtime(&event_time));
	static_cast<ostringstream&>(*this) << timebuf;
	break;
      case 'T':
        strftime(timebuf, 100, "%b %d %R %Z", localtime(&event_time));
	static_cast<ostringstream&>(*this) << timebuf;
	break;
      case 'o':
      {
	time_t tmp = co->get_last_online_time();
        strftime(timebuf, 100, "%b %d %R %Z", localtime(&tmp));
	static_cast<ostringstream&>(*this) << timebuf;
	break;
      }
      case 'm':
        static_cast<ostringstream&>(*this) << m_message_queue.get_contact_size(co);
	break;
      case 'r':
        sanecat(repeated ? "true" : "false");
        break;
      case '%':
        static_cast<ostringstream&>(*this) << '%';
	break;
      default:
        // Just output the %-substitution so they know something is wrong
	static_cast<ostringstream&>(*this) << '%';
	static_cast<ostringstream&>(*this) << c;
	break;
    }
    return (*this);
  }

  else if (got_special == '|') {
    // add next character to command string buffer
    cmd += c;
    return (*this);
  }

  else if (c == '%' || (line_start && c == '|')) {
    got_special = c;
    line_start = false;
    return (*this);
  }

  else {
    line_start = false;
    static_cast<ostringstream&>(*this) << c;
    return (*this);
  }
}

EventSubstituter& EventSubstituter::operator<<(const char *s) {
  while (*s)
    operator<<(*s++);
  return (*this);
}

EventSubstituter& EventSubstituter::operator<<(const string &s) {
  return operator<<(s.c_str());
}

string EventSubstituter::str()
{
  if (line_start == false && got_special == '|') execute();
  // ensure any non-terminated last lines of pipe are executed

  return ostringstream::str();
}

PipeExec::PipeExec() : fStdIn(NULL), fStdOut(NULL)
{ }

PipeExec::~PipeExec() { 
  Close();
}

bool PipeExec::Open(const char *shellcmd) {
  int pdes_out[2], pdes_in[2];

  if (pipe(pdes_out) < 0) return false;
  if (pipe(pdes_in) < 0) return false;

  switch (pid = fork())
  {
    case -1:                        /* Error. */
    {
      close(pdes_out[0]);
      close(pdes_out[1]);
      close(pdes_in[0]);
      close(pdes_in[1]);
      return false;
      /* NOTREACHED */
    }
  case 0:                         /* Child. */
    {
      if (pdes_out[1] != STDOUT_FILENO) {
        dup2(pdes_out[1], STDOUT_FILENO);
        close(pdes_out[1]);
      }
      
      close(pdes_out[0]);
      
      if (pdes_in[0] != STDIN_FILENO) {
        dup2(pdes_in[0], STDIN_FILENO);
        close(pdes_in[0]);
      }
      close(pdes_in[1]);
      system(shellcmd);
      _exit(0);   // exit() without the _ kills gtk+ dead.
                  // and they were playing sara with no h's favourite song
      /* NOTREACHED */
    }
  }

  /* Parent; assume fdopen can't fail. */
  fStdOut = fdopen(pdes_out[0], "r");
  close(pdes_out[1]);
  fStdIn = fdopen(pdes_in[1], "w");
  close(pdes_in[0]);

  // Set both streams to line buffered
#ifdef SETVBUF_REVERSED
  setvbuf(fStdOut, _IOLBF, (char*)NULL, 0);
  setvbuf(fStdIn, _IOLBF, (char*)NULL, 0);
#else
  setvbuf(fStdOut, (char*)NULL, _IOLBF, 0);
  setvbuf(fStdIn, (char*)NULL, _IOLBF, 0);
#endif

  return true;
}

void PipeExec::Read(char *buf, int size) {
  int pos = 0;
  int c;

  size--;
  while (((c = fgetc(fStdOut)) != EOF) && (pos < size)) buf[pos++] = (unsigned char)c;
  buf[pos] = '\0';
}

void PipeExec::Write(const char *buf) {
  fprintf(fStdIn, "%s", buf);
}

void PipeExec::CloseInput() {
  fclose(fStdIn);
  fStdIn = NULL;
}

void PipeExec::Close() {
   int r, pstat;
   struct timeval tv = { 0, 200000 };

   // Close the file descriptors
   if (fStdOut != NULL) fclose(fStdOut);
   if (fStdIn != NULL) fclose(fStdIn);
   fStdOut = fStdIn = NULL;

   if (pid == 0) return;

   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);

   // Return if child has exited or there was an inor
   if (r == pid || r == -1) return;
     
   // Give the process another .2 seconds to die
   select(0, NULL, NULL, NULL, &tv);
   
   // Still there?
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Try and kill the process
   if (kill(pid, SIGTERM) == -1) return;
   
   // Give it 1 more second to die
   tv.tv_sec = 1;
   tv.tv_usec = 0;
   select(0, NULL, NULL, NULL, &tv);
   
   // See if the child is still there
   r = waitpid(pid, &pstat, WNOHANG);
   if (r == pid || r == -1) return;

   // Kill, kill, keeeiiiiilllllll!!
   kill(pid, SIGKILL);
   // Now he will die for sure
   waitpid(pid, &pstat, 0);
}
