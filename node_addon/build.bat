
:: Builds ".cc" files specified in "binding.gyp" and all ".ts" files using the typescript-compiler
:: -arch = ia32 
node-gyp rebuild -release && tsc