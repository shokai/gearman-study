gearman study
=============

Require
=======

* gcc, g++
* Ruby 1.8.7

Install Dependencies
--------------------

    % gem install gearman-ruby json

    # for Mac OSX
    % brew install boost gearman

[Install OpenCV 1.0 on Mac](http://d.hatena.ne.jp/shokai/20110610/1307683786)


Run Gearman Server
==================

    % gearmand --verbose --port 7003

Build
=====

    % make -f Makefile.macosx
