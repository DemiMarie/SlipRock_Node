"use strict"
const sliprock = require('./build/Release/sliprock.node')
const assert = require('assert')
gc()
function main(index) {
   sliprock.new(index, function(err, val) {
      assert(err !== undefined)
      if (err != null) {
         gc()
         throw(err);
      }
      //val.close()
      if (index < 100) {
         main(index + 1)
      } else {
         gc()
         val.close()
      }
   })
}
main(0)
