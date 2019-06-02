module.exports = {
    plugins: ["promise", "jasmine", "jest"],
    env: {
        browser: true,
        node: true,
        es6: true,
        jasmine: true,
        jest: true,
        "jest/globals": true
    },
    parserOptions: {
        ecmaVersion: 2016,
        sourceType: "module",
        ecmaFeatures: {
            jsx: true,
            modules: true,
            experimentalObjectRestSpread: true
        }
    },
    extends: ["eslint:recommended", "plugin:promise/recommended", "plugin:jasmine/recommended", "plugin:jest/recommended", "prettier"],
    rules: {
        "max-len": ["warn", 140],
        "no-unused-vars": [
            "error",
            {
                args: "none"
            }
        ],
        "no-console": "off",
        "no-undef": "error", // disallow use of undeclared variables unless mentioned in a /*global */ block
        "no-undef-init": "error", // disallow use of undefined when initializing variables
        "no-undefined": "error", // disallow use of undefined variable (off by default)
        "no-unused-vars": "warn", // disallow declaration of variables that are not used in the code
        "no-use-before-define": "error", // disallow use of variables before they are defined
        "generator-star-spacing": 1,
        "array-bracket-spacing": 1,
        "arrow-parens": 1,
        "no-await-in-loop": 1,
        "no-mixed-spaces-and-tabs": "warn",
        eqeqeq: "warn"
    }
};
