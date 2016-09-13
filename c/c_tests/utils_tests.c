/**   LICENSE
* Copyright (c) 2016 Genome Research Ltd.
*
* Author: Cancer Genome Project cgpit@sanger.ac.uk
*
* This file is part of cgpBigWig.
*
* cgpBigWig is free software: you can redistribute it and/or modify it under
* the terms of the GNU Affero General Public License as published by the Free
* Software Foundation; either version 3 of the License, or (at your option) any
* later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
* FOR A PARTICULAR PURPOSE. See the GNU Affero General Public License for more
* details.
*
* You should have received a copy of the GNU Affero General Public License
* along with this program. If not, see <http://www.gnu.org/licenses/>.
*
*    1. The usage of a range of years within a copyright statement contained within
*    this distribution should be interpreted as being equivalent to a list of years
*    including the first and last year specified and all consecutive years between
*    them. For example, a copyright statement that reads ‘Copyright (c) 2005, 2007-
*    2009, 2011-2012’ should be interpreted as being identical to a statement that
*    reads ‘Copyright (c) 2005, 2007, 2008, 2009, 2011, 2012’ and a copyright
*    statement that reads ‘Copyright (c) 2005-2012’ should be interpreted as being
*    identical to a statement that reads ‘Copyright (c) 2005, 2006, 2007, 2008,
*    2009, 2010, 2011, 2012’."
*
*/
#include "minunit.h"
#include <string.h>
#include <utils.h>
#include <stdlib.h>


char *test_line_count(){
  char *file_path = "../test_data/volvox.fa.fai";
  int result = line_count(file_path);
  mu_assert(result==2,"Test line count");
  return NULL;
}

char *test_parse_file_name(){
  char *nodir = "test.a";
  char *onedir = "test_folder/test.b";
  char *multidir = "/one/two/three/test.c";
  char *dotdir = "./four/test.d";
  char *ddotdir = "../five/test.e";
  char *resdir = NULL;
  char *resname = NULL;
  resdir = calloc(1,sizeof(char) * 512);
  resname = calloc(1,sizeof(char) * 512);

  int check = parse_file_name(resdir, resname, nodir);
  mu_assert(strcmp(resname,"test.a")==0,"No directory fname");
  mu_assert(strcmp(resdir,"")==0,"No directory dir");
  mu_assert(check==1,"Return val 1");

  free(resdir);
  free(resname);
  resdir = calloc(1,sizeof(char) * 512);
  resname = calloc(1,sizeof(char) * 512);

  check = parse_file_name(resdir, resname, onedir);
  mu_assert(strcmp("test.b",resname)==0,"One directory fname");
  mu_assert(strcmp("test_folder/",resdir)==0,"One directory dir");
  mu_assert(check==1,"Return val 1");

  free(resdir);
  free(resname);
  resdir = calloc(1,sizeof(char) * 512);
  resname = calloc(1,sizeof(char) * 512);

  check = parse_file_name(resdir, resname, multidir);
  mu_assert(strcmp("test.c",resname)==0,"Multi directory fname");
  mu_assert(strcmp("/one/two/three/",resdir)==0,"Multi directory dir");
  mu_assert(check==1,"Return val 1");

  free(resdir);
  free(resname);
  resdir = calloc(1,sizeof(char) * 512);
  resname = calloc(1,sizeof(char) * 512);

  check = parse_file_name(resdir, resname, dotdir);
  mu_assert(strcmp("test.d",resname)==0,"Dot directory fname");
  mu_assert(strcmp("./four/",resdir)==0,"Dot directory dir");
  mu_assert(check==1,"Return val 1");

  free(resdir);
  free(resname);
  resdir = calloc(1,sizeof(char) * 512);
  resname = calloc(1,sizeof(char) * 512);

  check = parse_file_name(resdir, resname, ddotdir);
  mu_assert(strcmp("test.e",resname)==0,"Double Dot directory fname");
  mu_assert(strcmp("../five/",resdir)==0,"Double Dot directory dir");
  mu_assert(check==1,"Return val 1");

  free(resdir);
  free(resname);

  return NULL;
}


char *all_tests() {
   mu_suite_start();
   mu_run_test(test_line_count);
   mu_run_test(test_parse_file_name);
   return NULL;
}


RUN_TESTS(all_tests);