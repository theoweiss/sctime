sctime
Version 0.41b
5.8.2003

sctime provides a graphical interface which allows users to record the times used for
specific projects. 

DEVELOPER:
Florian Schmitt f.schmitt@science-computing.de

REPOSITORY:
This project uses CVS for version control. Use the following commands
to check it out:
cvs -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/sctime login
cvs -z3 -d:pserver:anonymous@cvs.sourceforge.net:/cvsroot/sctime co sctime


REQUIREMENTS:
The zeittools are required to access the database.

BUILDING:
A qt3 Library is required. Please check the paths in the apropriate 
Makefile before running make.

INSTALL:
All you need is the executable sctime, which you may copy
to a directory in your PATH variable.

TESTING:
To make really use of this project, you need to have two scripts
named "zeit" and "zeitkonten". By the script "zeit" the data can be checked into a database.
As a "zeit" script is not yet available under GPL, so you'll probably have to do without
database functionality (as long as you don't write your own wrapper).
The script "zeitkonten" should deliver a list of accounts, with each line consisting of the
following information:
account subaccount department accounting-nr comment
Department and comment are the only strings which may contain spaces.

DOCUMENTATION:
The source is documented in Docbook style. You might also want to check
the menu entry "Hilfe".

MAILING LISTS:
sctime-user@lists.sourceforge.net

CUSTOMER:
s+c and the public.

COPYRIGHT:
GNU Public License, see GPL

