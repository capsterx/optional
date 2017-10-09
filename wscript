from waflib import Context
import os
from waflib.TaskGen import taskgen_method
from waflib import Logs

def options(ctx):
  ctx.load('compiler_cxx')
  ctx.load('waf_unit_test')

def configure(ctx):
  ctx.load('compiler_cxx')
  ctx.load('waf_unit_test')
  ctx.env.append_value('INCLUDES', [os.path.abspath(os.path.join(os.getcwd(), Context.top_dir, "src"))])
  ctx.env.append_value('CXXFLAGS', "-std=c++14")
  ctx.env.append_value('CXXFLAGS', "-Wall")
  ctx.env.append_value('CXXFLAGS', "-Wextra")
  ctx.env.append_value('CXXFLAGS', "-Werror")
  ctx.env.append_value('CXXFLAGS', "-g")

def build(bld):
  @taskgen_method
  def add_test_results(self, tup):
    """Override and return tup[1] to interrupt the build immediately if a test does not run"""
    Logs.debug("ut: %r", tup)
    self.utest_result = tup
    try:
            self.bld.utest_results.append(tup)
    except AttributeError:
            self.bld.utest_results = [tup]
    if not tup[1] == 0:
      print tup[2]
    return tup[1]
  bld.recurse('tests')
