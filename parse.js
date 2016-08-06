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

    this.env.addFilter('jsvaltype', function(str) {
        if (!this.typeMapping[str] || !this.typeMapping[str].jsval) return 'undefined';
        return this.typeMapping[str].jsval;
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

NidiumIDL.prototype.generate = function(templatePath, outputPath) {
    for (var i = 0; i < this.tree.length; i++) {
        var obj = this.tree[i];

        switch(obj.type) {
            case 'interface':
                this.createInterface(templatePath, outputPath, obj);
                break;
            case 'dictionary':
                this.createDict(templatePath, outputPath, obj);
                break;
        }
    }
}


NidiumIDL.prototype.createInterface = function(templatePath, outputPath, obj)
{
    /*
        Scan for constructor
    */
    obj.ctor = false;

    var constructors = {
        maxArgs: 0,
        lst: []
    };

    for (var i = 0; i < obj.extAttrs.length; i++) {
        var attr = obj.extAttrs[i];
        if (attr.name == 'Constructor') {
            obj.ctor = true;
            constructors.lst.push(attr);
            constructors.maxArgs = Math.max(constructors.maxArgs, attr.arguments.length);
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

    obj.constructors = constructors;
    obj.operations = operations;

    //console.log(obj.operations);
    var interfaceHeader = this.env.render(templatePath + '/base_class.tpl.h', obj);
    var fileName = outputPath + "/base_" + obj.className + ".h";
    fs.writeFile(fileName, interfaceHeader, function(err) {
        if (err) {
            return console.log(err);
        }
        console.log("Wrote " + fileName);
    });
}

NidiumIDL.prototype.createDict = function(templatePath, outputPath, obj) {
    var dictHeader = this.env.render(templatePath + '/dict_class.h', obj);
    var fileName = outputPath + "/dict_" + obj.className + ".h";
    fs.writeFile(fileName, dictHeader, function(err) {
        if (err) {
            return console.log(err);
        }
        console.log("Wrote " + fileName);
    });
}

if (process.argv.length < 5) {
    console.log("Usage : " + process.argv[0] + " " + process.argv[1] + " idlfile templatepath outputpath\n");
} else {
    var idl = new NidiumIDL(process.argv[2]);
    idl.parse();
    //idl.printTree();
    idl.generate(process.argv[3], process.argv[4]);
}
