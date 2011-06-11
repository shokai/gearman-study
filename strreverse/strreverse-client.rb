#!/usr/bin/env ruby
# -*- coding: utf-8 -*-
require 'rubygems'
require 'gearman'

c = Gearman::Client.new(['localhost:7003'])
taskset = Gearman::TaskSet.new(c)

ARGV.each{|str|
  puts "add task #{str}"
  task = Gearman::Task.new('str_reverse', str)
  task.on_complete{|result|
    puts "return: #{str} => #{result}" # 文字列が逆になって返ってくる
  }
  taskset.add_task(task)
}
taskset.wait(100) # wait 100(sec)
