var WebIDL2 = require("webidl2");
var nunjucks = require('nunjucks');

var fs = require('fs');
var util = require("util");

var data = fs.readFileSync("./APE.idl", {encoding: "utf8"});

var tree = WebIDL2.parse(data);

console.log(util.inspect(tree, {colors: true, depth: 16}));


for (var i = 0; i < tree.length; i++) {
    var obj = tree[i];

    if (obj.type == 'interface') {
        
    }
}