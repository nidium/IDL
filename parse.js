var WebIDL2 = require("webidl2");
var nunjucks = require('nunjucks');

var fs = require('fs');
var util = require("util");

nunjucks.configure({
    trimBlocks: true,
    lstripBlocks: true
});

var data = fs.readFileSync("./APE.idl", {encoding: "utf8"});

var tree = WebIDL2.parse(data);

console.log(util.inspect(tree, {colors: true, depth: 16}));


for (var i = 0; i < tree.length; i++) {
    var obj = tree[i];

    switch(obj.type) {
        case 'interface':
            createInterface(obj);
            break;
        case 'dictionary':
            createDict(obj);
            break;
    }
}


function createInterface(obj)
{
    /*
        Scan for constructor
    */
    obj.ctor = false;
    for (var i = 0; i < obj.extAttrs.length; i++) {
        var attr = obj.extAttrs[i];
        console.log("extra", attr.name);
        if (attr.name == 'Constructor') {
            console.log("got ctor");
            obj.ctor = true;
            break;
        }
    }

    var interfaceHeader = nunjucks.render('templates/base_class.h', obj);
    console.log(interfaceHeader);
}

function createDict(obj) {
    var dictHeader = nunjucks.render('templates/dict_class.h', obj);

    console.log(dictHeader);
}