/*
   Copyright 2016 Nidium Inc. All rights reserved.
   Use of this source code is governed by a MIT license
   that can be found in the LICENSE file.
*/

var WebIDL2 = require("webidl2");
var nunjucks = require('nunjucks');

var fs = require('fs');
var util = require("util");

var mapping = {
    "boolean": {
        "c": "bool",
        "convert": "ToBoolean",
        "jsval": "boolean"
    },

    "cstring": {
        "c": "char *",
        "jsval": "string"
    },

    "byte": {
        "c": "int8_t",
        "convert": "ToInt16"
    },

    "octet": {
        "c": "uint8_t",
        "convert": "ToUint16"
    },

    "short": {
        "c": "int16_t",
        "convert": "ToInt16"
    },

    "unsigned short": {
        "c": "uint16_t",
        "convert": "ToUint16"
    },

    "long": {
        "c": "int32_t",
        "convert": "ToInt32"
    },

    "unsigned long": {
        "c": "uint32_t",
        "convert": "ToUint32"
    },

    "long long": {
        "c": "int64_t",
        "convert": "ToInt64"
    },

    "unsigned long long": {
        "c": "uint64_t",
        "convert": "ToUint64"
    },

    "float": {
        "c": "double",
        "convert": "ToNumber"
    },

    "double": {
        "c": "double",
        "convert": "ToNumber"
    }
}

var NidiumIDL = function(file, templatePath) {

    this.env = nunjucks.configure(templatePath, {
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
    this.typeMapping = mapping;
    //this.typeMapping = JSON.parse(fs.readFileSync("./types_mapping.json", {encoding: "utf8"}));
}

NidiumIDL.prototype.parse = function() {
    this.tree = WebIDL2.parse(this.data);
}

NidiumIDL.prototype.printTree = function() {
    console.log(util.inspect(this.tree, {colors: true, depth: 16}));
}


NidiumIDL.prototype.generate = function(outputPath, prefix) {
    var created_files = [];
    for (var i = 0; i < this.tree.length; i++) {
        var obj = this.tree[i];
        obj.getProperties = function(members) {
            var properties = []
            if ( members ) {
                for (var j = 0; j < members.length; j++){
                    if (members[j].type == 'attribute' ) {
                        properties.push(members[j]);
                    }
                }
            }
            return properties;
        };

        obj.getMethods = function(members, thatAreStatic) {
            var methods = []
            if ( members ) {
                for (var j = 0; j < members.length; j++){
                    var operation = members[j];
                    if (operation.type == 'operation' && operation.static == thatAreStatic ) {
                        methods.push(operation);
                    }
                }
            }
            return methods;
        };
        obj.hasAttr = function(extAttrs, key) {
            for( var j = 0; j < extAttrs.length; j++) {
                if (extAttrs[j].name == key) {
                    return extAttrs[j];
                }
            }
            return false;
        };
        switch(obj.type) {
            case 'interface':
                created_files = created_files.concat(this.createInterface(outputPath, obj, prefix));
                break;
            case 'dictionary':
                create_files = created_files.concat(this.createDict(outputPath, obj, prefix));
                break;
        }
    }

    return created_files;
}


NidiumIDL.prototype.createInterface = function(outputPath, obj, prefix)
{
    /*
        Scan for constructor
    */
    obj.ctor = false;
    var constructors = {
        maxArgs: 0,
        lst: []
    };
    var fileNames = [];

    for (var i = 0; i < obj.extAttrs.length; i++) {
        var attr = obj.extAttrs[i];
        if (attr.name == 'Constructor') {
            obj.ctor = true;
            constructors.lst.push(attr);
            var curArg = (attr.arguments) ? attr.arguments.length : 0;
            constructors.maxArgs = Math.max(constructors.maxArgs, curArg);
            continue;
        }

        if (attr.name == "className") {
            obj.className = attr.rhs.value;
        }
    }
    if (! obj.className) {
        obj.className = obj.name;
    }

    var operations = {};

    for (var i = 0; i < obj.members.length; i++) {
        var member = obj.members[i];
        if (member.type == 'operation') {
            if (!operations[member.name]) {
                var op = operations[member.name] = {lst: [], maxArgs: 0, name: member.name};
            }
            op.lst.push(member);
            op.maxArgs = Math.max(op.maxArgs, member.arguments.length);
        }
    }

    obj.constructors = constructors;
    obj.operations = operations;
    obj.prefix = prefix;

    console.log(obj);
    //console.log(obj.operations['foobar'].lst[0].idlType);
    var interfaceHeader = this.env.render('impl_class.tpl.h', obj);
    var fileNameHeader = outputPath + "/" + prefix + obj.className + ".h";
    fileNames.push(fileNameHeader);
    fs.writeFile(fileNameHeader, interfaceHeader, function(err) {
        if (err) {
            return console.log(err);
        }
    });
    var interfaceCode = this.env.render('impl_class.tpl.cpp', obj);
    var fileNameCode = outputPath + "/" + prefix + obj.className + ".cpp";
    fileNames.push(fileNameCode);
    fs.writeFile(fileNameCode, interfaceCode, function(err) {
        if (err) {
            return console.log(err);
        }
    });

    return fileNames;
}

NidiumIDL.prototype.createDict = function(outputPath, obj, prefix) {
    var fileNames = [];
    //console.log(obj);
    var dictHeader = this.env.render('dict_class.tpl.h', obj);
    var fileName = outputPath + "/" + prefix + obj.className + ".h";
    fileNames.push(fileName);
    fs.writeFile(fileName, dictHeader, function(err) {
        if (err) {
            return console.log(err);
        }
    });

    return fileNames;
}

if (process.argv.length < 6) {
    console.log("Usage : " + process.argv[0] + " " + process.argv[1] + " idlfile templatepath outputpath prefix\n");
} else {
    var idl = new NidiumIDL(process.argv[2], process.argv[3]);
    idl.parse();
    //idl.printTree();
    var created = idl.generate(process.argv[4], process.argv[5]);
    console.log(created.join(" "));
}
