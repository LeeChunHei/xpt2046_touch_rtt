import rtconfig
from building import *

cwd = GetCurrentDir()

if GetDepend(['PKG_USING_XPT2046_TOUCH']):
    src += Glob('drv_xpt2046.c')

if GetDepend(['PKG_XPT2046_USING_SAMPLE']):
    src += Glob('drv_sample.c')

CPPPATH = [cwd]
    
group = DefineGroup('xpt2046_touch', src, depend = [''], CPPPATH = CPPPATH)

Return('group')
