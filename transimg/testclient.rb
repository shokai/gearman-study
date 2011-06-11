#!/usr/bin/env ruby
# -*- coding: utf-8 -*-
require 'rubygems'
require 'gearman'

if ARGV.size < 1
  puts '画像へのパスが必要'
  puts 'ruby testclient.rb ~/path/to/images/*.png'
  exit 1
end

c = Gearman::Client.new(['localhost:7003'])
taskset = Gearman::TaskSet.new(c)

ARGV.sort{|a,b| a.split(/\//).last.to_i <=> b.split(/\//).last.to_i}.each{|name|
  name = "#{name},10,20,30\0"
  puts "add task #{name}"
  task = Gearman::Task.new("transimg", name)
  task.on_complete{|result|
    puts "return: #{name} => #{result}"
  }
  taskset.add_task(task)
}
taskset.wait(100) # wait 100(sec)

