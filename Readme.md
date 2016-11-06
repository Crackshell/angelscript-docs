# ScriptDocs

A class to generate a header file for Angelscript bindings. This can be used as a simple drop-in, and is used like this:

```C++
ScriptDocs docs(m_scriptEngine->m_engine);
docs.Write("Something.h");
```

You can then take the generated header file and generate a Doxygen site with it.

## License

This is licensed under the MIT license.
