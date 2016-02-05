 Pegit - Version Control System
===============================================================================

## Introduction
[Version control](https://en.wikipedia.org/wiki/Version_control) is a *system* that 
records changes to a file or set of files over time so that you can recall
specific versions later.

A VCS allows you to:
  + *revert* files back to a previous state,
  + *revert* the entire project back to a previous state,
  + *review* changes made over time,
  + *check* who last modified source code that might be causing a problem.

Using a VCS also means that if someone lose files, they can generally be
recovered easily. In addition, you get all this for very little overhead.

## Need and Requirement
As a part of their course plan/scheme students of computer science
branch are required to make coding projects may it be in any language C, C++,
Java, Python, HTML and so on. What the usual practice is, they make files
namely(.c ,.h, .py, .html ...), or ReadMe files and various directories.
They make single source code file and make all the changes in it and save it.
What happens in this practice is that they keep on losing the original code
and data. After making many additions and after editing the source code so many
times, say, one encounters a bug (a software bug is an error, flaw, failure or
fault in a computer program or system that causes it to produce an incorrect or
unexpected result, or to behave in unintended ways), it is very difficult to
trace the bug, meaning, it is very difficult to trace the part of the code which
is responsible for the unexpected behaviour.


## Manual Solution
Possible solution can be cloning the whole source code each time before making
even a minute change. This takes too much of users time (as code becomes larger)
and also has memory limitation. There is repetition of those files too in which
no change was made.


## Automated Solution (Version Control System)
Version control system is an automated system which keeps tracks of all the
changes in the repository of the project. System makes use of computer
processing and uses database to know which files have been edited, deleted or
newly created. When changes are made, the system will ask the developer if they
are to be committed(saved). Developer can give command to the system to save
them. Developer can retrieve any previous version of the project by accessing
the database. If developer makes a mistake, he can roll back to a previous
version. He can reproduce and understand a bug report on a past version of the
project's source code. He can also undo specific edits without losing all the
work that was done in the meanwhile. For any part of a file, he can determine
when and why it was ever edited.


## 1st Stage
All the information mentioned above will constitute the first stage of our
project.


## 2nd Stage
In the second stage of this project we will introduce networking concepts into
our project. Multiple users can work on same project. And it will enable
multiple people to simultaneously work on a single project. Each person edits
his or her own copy of the files and chooses when to share those changes with
the rest of the team. Thus, temporary or partial edits by one person do not
interfere with another person's work. It will also enable one person to use
multiple computers to work on a project. It will be able to integrate work done
simultaneously by different team members. In most cases, edits to different
files or even the same file will be combined without losing any work. In rare
cases, when two people make conflicting edits to the same line of a file, then
it will request human assistance in deciding what to do.


## 3rd Stage
In the third stage, we will introduce graphical user interface into our project.

_In this semester, we will work on first stage._

