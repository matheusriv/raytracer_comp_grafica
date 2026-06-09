# Ray Tracer - Computação Gráfica

Este é um projeto acadêmico em desenvolvimento para a disciplina de Computação Gráfica. O objetivo principal é implementar um motor de renderização 3D baseado na técnica de **Ray Tracing**, construído do zero em C++.

## Dependências
- Compilador com suporte nativo a **C++17** (GCC, Clang, etc.)
- **CMake** (versão 3.10 ou superior) ou **GNU Make**

## ⚙️ Como Compilar

**Usando CMake**
```bash
mkdir build
cd build
cmake ..
cmake --build .
```

## Como Executar

Após compilar com sucesso, o executável `raytmath` será gerado. Para iniciar a leitura e renderização, passe o caminho do arquivo de cena `.xml` desejado como argumento, por exemplo:

```bash
./raytmath ../scenes/interpolation.xml
```
*As imagens geradas serão salvas por padrão na pasta `results/`.*
