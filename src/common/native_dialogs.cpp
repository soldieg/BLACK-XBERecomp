#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>

#include "common/native_dialogs.h"

#include <array>

namespace native_dialogs {
namespace {

enum class Language { English, French, German, Italian, Spanish, Portuguese, Indonesian, Russian };

Language SystemLanguage() {
    switch (PRIMARYLANGID(GetUserDefaultUILanguage())) {
        case LANG_FRENCH: return Language::French;
        case LANG_GERMAN: return Language::German;
        case LANG_ITALIAN: return Language::Italian;
        case LANG_SPANISH: return Language::Spanish;
        case LANG_PORTUGUESE: return Language::Portuguese;
        case LANG_INDONESIAN: return Language::Indonesian;
        case LANG_RUSSIAN: return Language::Russian;
        default: return Language::English;
    }
}

constexpr size_t Index(Language language) { return static_cast<size_t>(language); }

struct LocalizedText {
    std::array<const wchar_t*, 8> values;
};

constexpr LocalizedText kTexts[] = {
    {{L"Put the game in the assets folder: its disc image (.iso), or default.xbe with GlobData.bin and the game's other files.",
       L"Placez le jeu dans le dossier assets : son image disque (.iso), ou default.xbe avec GlobData.bin et les autres fichiers du jeu.",
       L"Lege das Spiel in den Ordner assets: als Disc-Image (.iso) oder als default.xbe mit GlobData.bin und den übrigen Spieldateien.",
       L"Metti il gioco nella cartella assets: l'immagine disco (.iso), oppure default.xbe con GlobData.bin e gli altri file del gioco.",
       L"Coloca el juego en la carpeta assets: la imagen de disco (.iso), o default.xbe con GlobData.bin y los demás archivos del juego.",
       L"Coloque o jogo na pasta assets: a imagem do disco (.iso) ou default.xbe com GlobData.bin e os demais arquivos do jogo.",
       L"Letakkan game di folder assets: citra disk (.iso), atau default.xbe bersama GlobData.bin dan file game lainnya.",
       L"Поместите игру в папку assets: образ диска (.iso) либо default.xbe вместе с GlobData.bin и остальными файлами игры."}},
    {{L"The game failed to start. See the log in the logs folder.", L"Le jeu n'a pas pu démarrer. Consultez le journal dans le dossier logs.",
       L"Das Spiel konnte nicht gestartet werden. Weitere Informationen stehen im Protokollordner logs.",
       L"Impossibile avviare il gioco. Consulta il registro nella cartella logs.",
       L"No se pudo iniciar el juego. Consulta el registro en la carpeta logs.",
       L"Não foi possível iniciar o jogo. Consulte o log na pasta logs.",
       L"Game gagal dimulai. Lihat log di folder logs.",
       L"Не удалось запустить игру. См. журнал в папке logs."}},
    {{L"BLACK - Save Data Error", L"BLACK - Erreur des sauvegardes", L"BLACK - Speicherfehler", L"BLACK - Errore dati salvataggio",
       L"BLACK - Error de datos guardados", L"BLACK - Erro nos dados salvos", L"BLACK - Kesalahan data simpanan", L"BLACK — Ошибка сохранения"}},
    {{L"BLACK - Save Recovery", L"BLACK - Récupération de sauvegarde", L"BLACK - Speicherwiederherstellung",
       L"BLACK - Recupero salvataggio", L"BLACK - Recuperación de guardado", L"BLACK - Recuperação do save",
       L"BLACK - Pemulihan data simpanan", L"BLACK — Восстановление сохранения"}},
    {{L"Windows could not locate your local save-data folder. See the game log for details.",
       L"Windows n'a pas trouvé le dossier local des sauvegardes. Consultez le journal du jeu pour plus de détails.",
       L"Windows konnte den lokalen Speicherordner nicht finden. Weitere Informationen stehen im Spielprotokoll.",
       L"Windows non ha trovato la cartella locale dei salvataggi. Consulta il registro del gioco per i dettagli.",
       L"Windows no pudo encontrar la carpeta local de guardados. Consulta el registro del juego para más detalles.",
       L"O Windows não encontrou a pasta local dos saves. Consulte o log do jogo para ver os detalhes.",
       L"Windows tidak dapat menemukan folder data simpanan lokal. Lihat log game untuk detailnya.",
       L"Windows не удалось найти локальную папку сохранений. Подробности см. в журнале игры."}},
    {{L"The save-data folder could not be created. No game save was changed. See the game log.",
       L"Impossible de créer le dossier des sauvegardes. Aucune sauvegarde n'a été modifiée. Consultez le journal du jeu.",
       L"Der Speicherordner konnte nicht erstellt werden. Kein Spielstand wurde geändert. Siehe Spielprotokoll.",
       L"Impossibile creare la cartella dei salvataggi. Nessun salvataggio è stato modificato. Consulta il registro del gioco.",
       L"No se pudo crear la carpeta de guardados. No se modificó ningún guardado. Consulta el registro del juego.",
       L"Não foi possível criar a pasta de saves. Nenhum save foi alterado. Consulte o log do jogo.",
       L"Folder save tidak dapat dibuat. Tidak ada save yang diubah. Lihat log game.",
       L"Не удалось создать папку сохранений. Игровые сохранения не изменены. См. журнал игры."}},
    {{L"The existing save folder could not be checked. The game was stopped to protect your save.",
       L"Impossible de vérifier le dossier de sauvegarde existant. Le jeu a été arrêté pour protéger votre sauvegarde.",
       L"Der vorhandene Speicherordner konnte nicht geprüft werden. Das Spiel wurde zum Schutz des Spielstands beendet.",
       L"Impossibile controllare la cartella dei salvataggi esistente. Il gioco è stato arrestato per proteggere i dati.",
       L"No se pudo comprobar la carpeta de guardados existente. El juego se detuvo para proteger tus datos.",
       L"Não foi possível verificar a pasta de saves existente. O jogo foi interrompido para proteger seu save.",
       L"Folder save yang ada tidak dapat diperiksa. Game dihentikan untuk melindungi save Anda.",
       L"Не удалось проверить существующую папку сохранений. Игра остановлена для защиты ваших данных."}},
    {{L"Save recovery was cancelled. Existing files were left unchanged.",
       L"La récupération a été annulée. Les fichiers existants n'ont pas été modifiés.",
       L"Die Wiederherstellung wurde abgebrochen. Vorhandene Dateien blieben unverändert.",
       L"Recupero annullato. I file esistenti non sono stati modificati.",
       L"Se canceló la recuperación. Los archivos existentes no se modificaron.",
       L"A recuperação foi cancelada. Os arquivos existentes não foram alterados.",
       L"Pemulihan dibatalkan. File yang ada tidak diubah.",
       L"Восстановление отменено. Существующие файлы не изменены."}},
    {{L"The older save could not be safely restored. The game was stopped; original save copies remain in place. See the log.",
       L"Impossible de restaurer l'ancienne sauvegarde en toute sécurité. Le jeu a été arrêté ; les copies d'origine sont conservées. Consultez le journal.",
       L"Der ältere Spielstand konnte nicht sicher wiederhergestellt werden. Das Spiel wurde beendet; Originalkopien bleiben erhalten. Siehe Protokoll.",
       L"Impossibile ripristinare in sicurezza il vecchio salvataggio. Il gioco è stato arrestato e le copie originali sono conservate. Consulta il registro.",
       L"No se pudo restaurar de forma segura el guardado anterior. El juego se detuvo y las copias originales se conservaron. Consulta el registro.",
       L"Não foi possível restaurar o save antigo com segurança. O jogo foi interrompido e as cópias originais foram preservadas. Consulte o log.",
       L"Save lama tidak dapat dipulihkan dengan aman. Game dihentikan; salinan save asli tetap tersimpan. Lihat log.",
       L"Не удалось безопасно восстановить старое сохранение. Игра остановлена; исходные копии сохранены. См. журнал."}},
    {{L"A new save could not be safely initialized. No original save was intentionally deleted. See the log.",
       L"Impossible d'initialiser une nouvelle sauvegarde en toute sécurité. Aucune sauvegarde d'origine n'a été supprimée volontairement. Consultez le journal.",
       L"Ein neuer Spielstand konnte nicht sicher erstellt werden. Kein ursprünglicher Spielstand wurde absichtlich gelöscht. Siehe Protokoll.",
       L"Impossibile inizializzare in sicurezza un nuovo salvataggio. Nessun salvataggio originale è stato eliminato intenzionalmente. Consulta il registro.",
       L"No se pudo inicializar un guardado nuevo de forma segura. No se eliminó ningún guardado original intencionalmente. Consulta el registro.",
       L"Não foi possível inicializar um novo save com segurança. Nenhum save original foi apagado intencionalmente. Consulte o log.",
       L"Save baru tidak dapat dibuat dengan aman. Tidak ada save asli yang sengaja dihapus. Lihat log.",
       L"Не удалось безопасно создать новое сохранение. Исходные сохранения не удалялись намеренно. См. журнал."}},
    {{L"The save HDD failed its read check. The game was stopped to protect your data.",
       L"Le disque de sauvegarde n'a pas passé la vérification de lecture. Le jeu a été arrêté pour protéger vos données.",
       L"Die Lesprüfung des Speicher-HDDs ist fehlgeschlagen. Das Spiel wurde zum Schutz Ihrer Daten beendet.",
       L"Il controllo di lettura dell'HDD dei salvataggi non è riuscito. Il gioco è stato arrestato per proteggere i dati.",
       L"Falló la comprobación de lectura del HDD de guardado. El juego se detuvo para proteger tus datos.",
       L"A verificação de leitura do HDD de saves falhou. O jogo foi interrompido para proteger seus dados.",
       L"Pemeriksaan baca HDD save gagal. Game dihentikan untuk melindungi data Anda.",
       L"Проверка чтения HDD сохранений не пройдена. Игра остановлена для защиты ваших данных."}},
};

const wchar_t* Get(TextId id) { return kTexts[static_cast<size_t>(id)].values[Index(SystemLanguage())]; }

struct RecoveryCopy {
    std::array<const wchar_t*, 8> intro;
    std::array<const wchar_t*, 8> fresh;
    std::array<const wchar_t*, 8> one_yes;
    std::array<const wchar_t*, 8> one_no;
    std::array<const wchar_t*, 8> cancel;
    std::array<const wchar_t*, 8> older;
    std::array<const wchar_t*, 8> multi_yes;
    std::array<const wchar_t*, 8> multi_cancel;
    std::array<const wchar_t*, 8> multiple;
    std::array<const wchar_t*, 8> none;
};

constexpr RecoveryCopy kRecovery = {
    {{L"Save data could not be safely loaded:", L"Impossible de charger la sauvegarde en toute sécurité :", L"Der Spielstand konnte nicht sicher geladen werden:", L"Impossibile caricare il salvataggio in sicurezza:", L"No se pudo cargar el guardado de forma segura:", L"Data save tidak dapat dimuat dengan aman:", L"Não foi possível carregar o save com segurança:", L"Не удалось безопасно загрузить сохранение:"}},
    {{L"The original files will be preserved before starting a new profile.", L"Les fichiers d'origine seront conservés avant la création d'un nouveau profil.", L"Die Originaldateien werden vor einem neuen Profil gesichert.", L"I file originali saranno conservati prima di creare un nuovo profilo.", L"Los archivos originales se conservarán antes de crear un perfil nuevo.", L"File asli akan disimpan sebelum membuat profil baru.", L"Os arquivos originais serão preservados antes de criar um novo perfil.", L"Исходные файлы будут сохранены перед созданием нового профиля."}},
    {{L"Yes: restore the readable older save that passed structural and checksum checks.", L"Oui : restaurer l'ancienne sauvegarde lisible dont la structure et les sommes de contrôle sont valides.", L"Ja: den lesbaren älteren Spielstand mit gültiger Struktur und Prüfsumme wiederherstellen.", L"Sì: ripristinare il vecchio salvataggio leggibile con struttura e checksum validi.", L"Sí: restaurar el guardado anterior legible que superó las comprobaciones de estructura y suma de verificación.", L"Ya: pulihkan save lama yang dapat dibaca dan lolos pemeriksaan struktur serta checksum.", L"Sim: restaurar o save antigo legível que passou pelas verificações de estrutura e checksum.", L"Да: восстановить читаемое старое сохранение с проверенной структурой и контрольными суммами."}},
    {{L"No: preserve the damaged HDD as a backup and start a new game.", L"Non : conserver le HDD endommagé en sauvegarde et commencer une nouvelle partie.", L"Nein: das beschädigte HDD als Sicherung behalten und ein neues Spiel starten.", L"No: conservare l'HDD danneggiato come backup e iniziare una nuova partita.", L"No: conservar el HDD dañado como copia de seguridad y empezar una partida nueva.", L"Tidak: simpan HDD yang rusak sebagai cadangan lalu mulai game baru.", L"Não: preservar o HDD danificado como backup e começar um jogo novo.", L"Нет: сохранить поврежденный HDD в резервной копии и начать новую игру."}},
    {{L"Cancel: exit without changing any save.", L"Annuler : quitter sans modifier aucune sauvegarde.", L"Abbrechen: beenden, ohne Spielstände zu ändern.", L"Annulla: uscire senza modificare alcun salvataggio.", L"Cancelar: salir sin cambiar ningún guardado.", L"Batal: keluar tanpa mengubah data simpanan.", L"Cancelar: sair sem alterar nenhum save.", L"Отмена: выйти без изменения сохранений."}},
    {{L"Older save copy:", L"Ancienne copie de sauvegarde :", L"Ältere Sicherung:", L"Vecchia copia del salvataggio:", L"Copia de guardado anterior:", L"Salinan save lama:", L"Cópia antiga do save:", L"Старая копия сохранения:"}},
    {{L"Yes: preserve this save and start a new game.", L"Oui : conserver cette sauvegarde et commencer une nouvelle partie.", L"Ja: diesen Spielstand sichern und ein neues Spiel starten.", L"Sì: conservare questo salvataggio e iniziare una nuova partita.", L"Sí: conservar este guardado y empezar una partida nueva.", L"Ya: simpan save ini lalu mulai game baru.", L"Sim: preservar este save e começar um jogo novo.", L"Да: сохранить этот файл и начать новую игру."}},
    {{L"No or Cancel: exit without changing saves.", L"Non ou Annuler : quitter sans modifier les sauvegardes.", L"Nein oder Abbrechen: beenden, ohne Spielstände zu ändern.", L"No o Annulla: uscire senza modificare i salvataggi.", L"No o Cancelar: salir sin cambiar los guardados.", L"Tidak atau Batal: keluar tanpa mengubah save.", L"Não ou Cancelar: sair sem alterar os saves.", L"Нет или Отмена: выйти без изменения сохранений."}},
    {{L"More than one readable older save exists, so the game will not guess which one to restore:", L"Plusieurs anciennes sauvegardes lisibles existent ; le jeu ne choisira pas laquelle restaurer :", L"Es gibt mehrere lesbare ältere Spielstände. Das Spiel wählt keinen davon automatisch aus:", L"Esistono più vecchi salvataggi leggibili; il gioco non sceglierà quale ripristinare:", L"Hay varios guardados anteriores legibles; el juego no elegirá cuál restaurar:", L"Ada lebih dari satu save lama yang dapat dibaca, jadi game tidak akan memilih sendiri:", L"Há mais de um save antigo legível, então o jogo não vai escolher um automaticamente:", L"Найдено несколько читаемых старых сохранений; игра не будет выбирать одно автоматически:"}},
    {{L"No readable older copy passed the structural checks. Progress from this damaged save cannot be safely reconstructed.", L"Aucune ancienne copie lisible n'a passé les contrôles de structure. La progression de cette sauvegarde endommagée ne peut pas être restaurée en toute sécurité.", L"Keine lesbare ältere Kopie hat die Strukturprüfung bestanden. Der Fortschritt aus diesem beschädigten Spielstand kann nicht sicher rekonstruiert werden.", L"Nessuna vecchia copia leggibile ha superato i controlli strutturali. I progressi di questo salvataggio danneggiato non possono essere ricostruiti in sicurezza.", L"Ninguna copia anterior legible superó las comprobaciones de estructura. No se puede reconstruir de forma segura el progreso de este guardado dañado.", L"Tidak ada salinan lama yang lolos pemeriksaan struktur. Progres dari save yang rusak ini tidak dapat dipulihkan dengan aman.", L"Nenhuma cópia antiga legível passou nas verificações estruturais. Não é seguro reconstruir o progresso deste save danificado.", L"Ни одна читаемая старая копия не прошла проверку структуры. Невозможно безопасно восстановить прогресс из поврежденного сохранения."}}
};

}  // namespace

std::wstring Text(TextId id) { return Get(id); }

std::wstring GameReleaseWarning(const std::wstring& release_name) {
    static constexpr std::array<const wchar_t*, 8> before = {
        L" is a different release of the game than this build was made from, and will most likely crash.\n\nStart it anyway?",
        L" est une version différente de celle utilisée pour cette compilation et risque fortement de planter.\n\nLa démarrer quand même ?",
        L" ist eine andere Spielversion als die für diesen Build und wird wahrscheinlich abstürzen.\n\nTrotzdem starten?",
        L" è una versione diversa da quella usata per questa build e probabilmente si bloccherà.\n\nAvviarla comunque?",
        L" es una versión distinta de la usada para esta compilación y probablemente se bloqueará.\n\n¿Iniciarla de todos modos?",
        L" es uma versão diferente da usada nesta compilação e provavelmente vai falhar.\n\nIniciar mesmo assim?",
        L" adalah versi berbeda dari yang digunakan untuk build ini dan kemungkinan besar akan crash.\n\nTetap jalankan?",
        L" — другая версия игры, не соответствующая этой сборке; вероятнее всего, произойдет сбой.\n\nВсе равно запустить?"};
    return release_name + before[Index(SystemLanguage())];
}

std::wstring SaveRecoveryMessage(const std::filesystem::path& damaged_hdd,
                                const std::vector<std::filesystem::path>& verified_older_hdds) {
    size_t i = Index(SystemLanguage());
    if (i == 5) i = 6;  // recovery strings store Indonesian before Portuguese
    else if (i == 6) i = 5;
    std::wstring message = kRecovery.intro[i] + std::wstring(L"\n") + damaged_hdd.wstring() + L"\n\n" +
                           kRecovery.fresh[i] + L"\n\n";
    if (verified_older_hdds.size() == 1) {
        message += kRecovery.one_yes[i] + std::wstring(L"\n") + kRecovery.one_no[i] + L"\n" + kRecovery.cancel[i] +
                   L"\n\n" + kRecovery.older[i] + L" " + verified_older_hdds.front().wstring();
        return message;
    }
    message += kRecovery.multi_yes[i] + std::wstring(L"\n") + kRecovery.multi_cancel[i];
    if (!verified_older_hdds.empty()) {
        message += L"\n\n" + std::wstring(kRecovery.multiple[i]);
        for (const auto& older : verified_older_hdds) message += L"\n" + older.wstring();
    } else {
        message += L"\n\n" + std::wstring(kRecovery.none[i]);
    }
    return message;
}

}  // namespace native_dialogs
