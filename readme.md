# Scenario Utilities

Command line tools to assist in unopinionated porting of classic Marathon 2 and Marathon Infinity scenarios.

## Compiling

There is no auto-build system. Just a Makefile. You will need C++11 and a fairly modern version of Boost (1.74 definitely works).

## fuxdiff

Diffs two Fux! state files and outputs to stdout MML that would achieve the same effect in Aleph One. To create a state file, open the patched engine in Fux! and select Export from the file menu.

## strdiff

Diffs two MacBinary-encoded Marathon Infinity-derived engines and outputs the string customizations in the second engine.
