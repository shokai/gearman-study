#!/usr/bin/env ruby
# -*- coding: utf-8 -*-
require 'rubygems'
require 'gearman'
require 'json'

if ARGV.size < 1
  puts '画像へのパスが必要'
  puts 'ruby testclient.rb ~/path/to/images/*.png'
  exit 1
end

c = Gearman::Client.new(['localhost:7003'])
taskset = Gearman::TaskSet.new(c)

ARGV.sort{|a,b| a.split(/\//).last.to_i <=> b.split(/\//).last.to_i}.each{|name|
  puts "add task #{name}"
  task = Gearman::Task.new("img_size", name+"\0")
  task.on_complete{|result|
    result = JSON.parse(result)
    print "return: #{name} =>"
    p result
  }
  taskset.add_task(task)
}
taskset.wait(100) # wait 100(sec)

