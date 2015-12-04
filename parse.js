var WebIDL2 = require("webidl2");
var nunjucks = require('nunjucks');

var fs = require('fs');
var util = require("util");


var NidiumIDL = function(file) {

    this.env = nunjucks.configure({
        trimBlocks: true,
        lstripBlocks: true
    });

    this.loadConf();

    this.env.addFilter('ctype', function(str) {
        if (!this.typeMapping[str] || !this.typeMapping[str].c) return str;
        return this.typeMapping[str].c;
    }.bind(this));

    this.env.addFilter('convert', function(str) {
        if (!this.typeMapping[str] || !this.typeMapping[str].convert) return str;
        return this.typeMapping[str].convert;
    }.bind(this));

    this.data = fs.readFileSync(file, {encoding: "utf8"});
}

NidiumIDL.prototype.loadConf = function() {
    this.typeMapping = JSON.parse(fs.readFileSync("./types_mapping.json", {encoding: "utf8"}));
}

NidiumIDL.prototype.parse = function() {
    this.tree = WebIDL2.parse(this.data);
}

NidiumIDL.prototype.printTree = function() {
    console.log(util.inspect(this.tree, {colors: true, depth: 16}));
}

NidiumIDL.prototype.generate = function() {
    for (var i = 0; i < this.tree.length; i++) {
        var obj = this.tree[i];

        switch(obj.type) {
            case 'interface':
                this.createInterface(obj);
                break;
            case 'dictionary':
                this.createDict(obj);
                break;
        }
    }
}


NidiumIDL.prototype.createInterface = function(obj)
{
    /*
        Scan for constructor
    */
    obj.ctor = false;
    for (var i = 0; i < obj.extAttrs.length; i++) {
        var attr = obj.extAttrs[i];
        if (attr.name == 'Constructor') {
            console.log("got ctor");
            obj.ctor = true;
            continue;
        }

        if (attr.name == "className") {
            obj.className = attr.rhs.value;
        }
    }

    var operations = {};

    for (var i = 0; i < obj.members.length; i++) {
        var member = obj.members[i];
        if (member.type != 'operation') continue;

        if (!operations[member.name]) {
            var op = operations[member.name] = {lst: [], maxArgs: 0, name: member.name};
        }

        op.lst.push(member);
        op.maxArgs = Math.max(op.maxArgs, member.arguments.length);
    }

    obj.operations = operations;

    console.log(obj.operations);

    var interfaceHeader = this.env.render('templates/base_class.h', obj);
    console.log(interfaceHeader);
}

NidiumIDL.prototype.createDict = function(obj) {
    var dictHeader = this.env.render('templates/dict_class.h', obj);

    console.log(dictHeader);
}



var idl = new NidiumIDL("./APE.idl");
idl.parse();
idl.printTree();
idl.generate();