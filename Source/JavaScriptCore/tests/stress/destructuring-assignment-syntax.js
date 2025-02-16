
function testSyntax(script) {
    try {
        eval(script);
    } catch (error) {
        if (error instanceof SyntaxError)
            throw new Error("Bad error: " + String(error));
    }
}

function testSyntaxError(script, message) {
    var error = null;
    try {
        eval(script);
    } catch (e) {
        error = e;
    }
    if (!error)
        throw new Error("Expected syntax error not thrown");

    if (String(error) !== message)
        throw new Error("Bad error: " + String(error));
}

testSyntax("({ a: this.a } = {})");
testSyntax("({ a: this['a'] } = {})");
testSyntax("({ a: this[\"a\"] } = {})");
testSyntax("[this.a ] = []");
testSyntax("[this['a']] = []");
testSyntax("[this[0]] = []");
testSyntax("[...this[0]] = []");
testSyntax("[...[function f() {}.prop]] = []");
testSyntax("[...[{prop: 1}.prop]] = []");
testSyntax("[...[this[0], ...this[1]]] = []");
testSyntax("({ a: obj.a } = {})");
testSyntax("({ a: obj['a'] } = {})");
testSyntax("({ a: obj[\"a\"] } = {})");
testSyntax("({ a: function() {}['prop'] } = {})");
testSyntax("({ a: {prop: 1}.prop } = {})");
testSyntax("[obj.a ] = []");
testSyntax("[obj['a']] = []");
testSyntax("[obj[0]] = []");
testSyntax("[function(){}.prop] = []");
testSyntax("[{prop: 1}.prop] = []");


testSyntaxError("[...c = 1] = []", "SyntaxError: Unexpected token '='. Expected a closing ']' following a rest element destructuring pattern.");
testSyntaxError("[...c, d] = []", "SyntaxError: Unexpected token ','. Expected a closing ']' following a rest element destructuring pattern.");
testSyntaxError("[this] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[th\\u{69}s] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[function() {}] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("['string'] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[123] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[true] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[tru\\u0065] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[false] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[f\\u0061lse] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[null] = []", "SyntaxError: Invalid destructuring assignment target.");
testSyntaxError("[n\\u{75}ll] = []", "SyntaxError: Invalid destructuring assignment target.");
