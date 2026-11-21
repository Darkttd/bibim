# NUnit 3.14.0

Used by `tools/Bibim.Toolkit.Tests`. The files come from the NuGet packages, unchanged:

| File | Package | Path in the package |
|---|---|---|
| `nunit.framework.dll` | [NUnit 3.14.0](https://www.nuget.org/packages/NUnit/3.14.0) | `lib/net45/` |
| `nunitlite.dll` | [NUnitLite 3.14.0](https://www.nuget.org/packages/NUnitLite/3.14.0) | `lib/net45/` |
| `LICENSE.txt`, `NOTICES.txt` | NUnit 3.14.0 | package root |

SHA-256 of the packages: NUnit `0ae3ffab91e8bcf59f016dc2b72fd0c518bb5698f2909dd30cbab94c4348e8a6`,
NUnitLite `75c5f1ded2ca9d510cad569d2f60d9243ebffab502c35840406333bedc4a9696`.
SHA-256 of the files here: `nunit.framework.dll` `1d88aadc5b1ab74027b8fe36aeabdae8bf50f1ceee4fa792db6117b46321dd74`,
`nunitlite.dll` `e550649611e3f10d5e91d30ec60fbed1d887a1ede12c65816201ea864409fec8`.

The .NET Framework build of NUnit 3.14 needs no other assemblies. NUnit 4.4's (net462) also needs
System.Threading.Tasks.Extensions and System.ValueTuple. License: MIT (`LICENSE.txt`).
