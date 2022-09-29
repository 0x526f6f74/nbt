# nbt

> NOTE: This is not production software and I do not recommend anyone to use this library.

Simple library for decoding and encoding MCBE flavoured NBT files.

## Example

*Reads a `.mcstructure` file, prints `format_version` and encodes the structure to another file.*

```c++
nbt::NBT structure {std::ifstream {"example.mcstructure", std::ios::binary}};

std::cout << structure["format_version"].as<nbt::TagInt>() << '\n';

structure.encode(std::ofstream {"encoded.mcstructure", std::ios::binary});
```
