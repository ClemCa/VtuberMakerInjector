# Vtuber Maker Keybind Injector

This small executable and accompanying DLL allow you to use Vtuber Maker keybinds in an automated way without affecting other parts of your system, such as games. The executable uses DLL injection to pass custom keybinds to Vtuber Maker.

The injector checks if the DLL is already injected into the Vtuber Maker process. If `refresh` is set to `true`, the injector will remove the DLL and inject it anew. If `refresh` is set to `false`, the injector will skip the injection step.

## Usage

To use the injector, you must specify a key as the first argument. If no key is specified, the default key will be 't'. You can also specify the path to the DLL as the second argument, or use the default path by leaving this argument blank. To refresh the keybinds, specify '1' as the third argument.

For example:

```  
./injector.exe f C:\path\to\keyboard.dll 1  
```

This will use the key 'f' for the keybinds, load the DLL from the specified path, and refresh the keybinds by removing and re-injecting the DLL.

```  
./injector.exe C:\path\to\keyboard.dll  
```

This will use the default key 't' for the keybinds, load the DLL from the specified path, and skip the injection step if the DLL is already present in the Vtuber Maker process.

```  
./injector.exe  
```

This will use the default key 't' for the keybinds, use the default path for the DLL, and skip the injection step if the DLL is already present in the Vtuber Maker process.

## Note

Please ensure that you have the necessary permissions to inject DLLs into the Vtuber Maker process. The injector is provided as-is and the author is not responsible for any damages or unintended consequences that may occur from its use. Use at your own risk.
