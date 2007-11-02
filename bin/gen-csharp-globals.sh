#!/bin/sh
#   C# Globals Symbols Collector
#   Copyright (C) 2007 by Ronie Salgado <roniesalg@gmail.com>
#
#   This program is free software; you can redistribute it and/or modify
#   it under the terms of the GNU General Public License as published by
#   the Free Software Foundation; either version 2 of the License, or
#   (at your option) any later version.
#
#   This program is distributed in the hope that it will be useful,
#   but WITHOUT ANY WARRANTY; without even the implied warranty of
#   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#   GNU General Public License for more details.
#
#   You should have received a copy of the GNU General Public License
#   along with this program; if not, write to the Free Software
#   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

# Extract the static procedure from an input, and write proxys for it.
# It assumes that the input is the first argument, and that the module name is
# the second argument. It assumes that the module is where are located the
# the static procedures.
extract_static_procedures()
{
  INPUT=$1
  MOD=$2
  INPUTDIR=$3
  I=0
  egrep -e "public[ ]+static[^\=]+\(.*\)" $INPUT |
  sed -e 's/{/ /g
	  s/(/ ( /g
	  s/)/ )/g
	  s/,/ , /g
  ' |
  sort -n |
  awk '
       {
	 ORS="";
	 print "   ";
	 for(i = 1; i < 4; i++)
	 {
	   print " " $i;
	 }
         if($4 == "GetSCFPointer" ||
	    $4 == "SetSCFPointer")
	 {
	   print " " Module "_" $4 ;
	 }
	 else
	 {
	   print " " $4 ;
	 }
	 print "(";
	
	 for(i = 6; i < NF; i++)
	 {
	   if($i != "," && i!= 6)
	    print " ";
	   print $i;
	 }
	 print ")\n";
       }
       {
	 print "    {\n" 
       }        
       {
	 if($3!="void")
	   print "      return ";
	 else
	   print "      ";

	 print Module "." $4 "(";
	 for(i = 7; i < NF; i+=3)
	 {
	   if(i > 7)
	     print ", ";
	   print $i;
	 }
	 ORS="\n";
	 print ");";
      }
      {
	 print "    }";
      }
     ' Module="$MOD"
}

# Extract the constants from an input and write it back
extract_constants()
{
  INPUT=$1
  MOD=$2
  INPUTDIR=$3

  egrep -e "public[ 	]+[(const)(static)].+\=.+;" $INPUT |
  awk ' { print "  " $0; } '
}

# Prints a bulk of comments about the module
print_module_header()
{
  MODULE=$1

  echo "    //==================================================="
  echo "    // Static and public procedures from $MODULE"
  echo "    //==================================================="
}

# Prints a bulk of comments about the module constants
print_module_const_header()
{
  MODULE=$1

  echo "    //==================================================="
  echo "    // Constants from $MODULE"
  echo "    //==================================================="
}

# Print the header of the generated file. Here is where must be the copyright
# notice of the generated file, and aditional supports things.
print_header()
{
  cat << !
/*
    Copyright (C) 2007 by Ronie Salgado <roniesalg@gmail.com>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public
    License along with this library; if not, write to the Free
    Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
*/

// WARNING: This file was automatically generated by joincshmodules.sh and is not
//          intended for manual editing
using System;
using System.Runtime.InteropServices;

!

  echo "namespace $NAMESPACE"
  echo "{"
  echo "  public class $CLASSNAME"
  echo "  {"
}

print_footer()
{
  echo "  }"
  echo "}; //namespace $NAMESPACE"
}

print_usage()
{
  echo "joincshmodules.sh namespace classname outputfile module1 [module2] ..."
}

if [ "$#" -lt 4 ] ; then
  print_usage
  exit 1
fi

NAMESPACE="$1"
CLASSNAME="$2"
OUTPUT="$3"
FILESDIR="`dirname $OUTPUT`"

declare -a MODULES

i="4"
ARGV=($*)
while [ "$i" -le "$#" ]
do

  MODULES[$[i-3]]="${ARGV[$[i-1]]}"
  i=$[$i+1]
done



# Prints a bulk of comment at the head of the generated file, the using
# directives, the namespace and the class which will contain our symbols
print_header > $OUTPUT

for MOD in "${MODULES[@]}" ; do
  # Extract and process $MOD public and static procedures
  echo "Working in module: $MOD"
  echo "Extracting globals procedures"
  # Prints a block of comments which do more easy to identify the partd
  # generated for a determined module
  print_module_header ${MOD} >> $OUTPUT
  # Do the dirty work
  extract_static_procedures $FILESDIR/${MOD}.cs $MOD >> $OUTPUT
  echo >> $OUTPUT

  # Extract and process $MOD public constants and static readonly variables
  echo "Extracting globals constants"
  # Equals than print_module_header, but the comments identify the constants
  # of the same module
  print_module_const_header ${MOD} >> $OUTPUT
  # Do the dirty work with the constants
  extract_constants $FILESDIR/${MOD}.cs $MOD >> $OUTPUT
  echo >> $OUTPUT
done

# Prints the footer of the generated file. The footer closes the class and
# namespace blocks
print_footer >> $OUTPUT

