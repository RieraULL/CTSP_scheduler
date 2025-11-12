# Guía de Instalación

Esta guía cubre la instalación y compilación del proyecto con dos alternativas de solver:

- CLP (COIN-OR) – open-source, recomendado para LP
- CPLEX (IBM ILOG) – comercial, recomendado si ya lo usas

Ambas opciones funcionan mediante presets de CMake.

## Requisitos comunes

- CMake 3.10+
- Compilador C++ con soporte C++14
- Git (opcional, para clonar el repo)

Clona el repositorio:

```bash
git clone https://github.com/RieraULL/CTSP_scheduler.git
cd CTSP_scheduler
```

---

## Opción A: CLP (Open-Source)

El proyecto puede construir con CLP sin necesidad de `ClpConfig.cmake` gracias a un mecanismo de descubrimiento automático de headers y librerías del sistema.

### 1) Instalar dependencias (Debian/Ubuntu)

```bash
sudo apt-get update
sudo apt-get install -y \
  coinor-libclp-dev coinor-libcoinutils-dev coinor-libosi-dev \
  pkgconf
```

Verificación rápida (opcional):

```bash
ls /usr/include/coin | grep -E "^Clp.*hpp$" | head
ls /usr/lib/x86_64-linux-gnu/libClp.so
```

### 2) Configurar y compilar (preset clp)

```bash
cmake --preset clp
cmake --build --preset clp -j
```

### 3) Ejecutar (CLP)

```bash
./build-clp/bin/ctsp_scheduler ctsp2 \
  input/bayg29_p5_f90_lL.contsp \
  input/bayg29_p5_f90_lL.sol \
  output/bayg29_p5_f90_lL.sched.json
```

### Notas y resolución de problemas

- Si CMake no encuentra `ClpConfig.cmake`, el proyecto hace fallback automático buscando headers en `/usr/include/coin` y librerías en `/usr/lib/x86_64-linux-gnu`.
- CLP es LP-only. Si tu modelo tiene variables enteras, se resolverá la relajación LP. Para MIP integra CBC (COIN-OR Branch-and-Cut).
- Instalación personalizada (si instalaste CLP fuera de rutas estándar):

```bash
# Si tienes ClpConfig.cmake
cmake --preset clp -DClp_DIR=/usr/local/lib/cmake/Clp

# O añade el prefijo
cmake --preset clp -DCMAKE_PREFIX_PATH=/usr/local
```

---

## Opción B: CPLEX (Comercial)

Asegúrate de tener IBM ILOG CPLEX instalado localmente. Luego apunta la variable `CPX_PATH` en `src/util/CMakeLists.txt` a tu instalación.

### 1) Ajustar ruta CPLEX

Edita `src/util/CMakeLists.txt` y cambia:

```cmake
set (CPX_PATH /ruta/a/tu/instalacion/CPLEX_Studio221)
```

### 2) Configurar y compilar (preset cplex)

```bash
cmake --preset cplex
cmake --build --preset cplex -j
```

### 3) Ejecutar (CPLEX)

```bash
./build-cplex/bin/ctsp_scheduler ctsp2 \
  input/bayg29_p5_f90_lL.contsp \
  input/bayg29_p5_f90_lL.sol \
  output/bayg29_p5_f90_lL.sched.json
```

---

## Depuración en VS Code (opcional)

Incluimos configuraciones en `.vscode/launch.json` para ejecutar `ctsp_scheduler` con los argumentos de ejemplo. Hay variantes para `build-cplex` y `build-clp`.

---

## Consejos

- Si cambias de solver (CLP/CPLEX), vuelve a configurar con el preset correspondiente.
- Si ves errores de link con CLP, confirma la presencia de `libClp.so`, `libClpSolver.so`, `libCoinUtils.so`, `libOsi.so`, `libOsiClp.so` en `/usr/lib/x86_64-linux-gnu/`.
- Para documentación del código (Doxygen), revisa la sección correspondiente en el `README.md`.
