@echo off

@if exist "C:\Program Files\e-Gulden\egulden-qt.exe" (
    echo e-Gulden Core gevonden!
    echo We starten e-Gulden nu in een speciale modus. U hoeft dit programma niet nog eens uit te voeren.
    echo.
    echo Uw transactiegeschiedenis is nog niet zichtbaar. Geef de wallet de tijd om bij te werken.
    echo.
    "C:\Program Files\e-Gulden\egulden-qt.exe" -zapwallettxes=1
) else if exist "C:\Program Files (x86)\e-Gulden\egulden-qt.exe" (
    echo e-Gulden Core gevonden!
    echo We starten e-Gulden nu in een speciale modus. U hoeft dit programma niet nog eens uit te voeren.
    echo.
    echo Uw transactiegeschiedenis is nog niet zichtbaar. Geef de wallet de tijd om bij te werken.
    echo.
    "C:\Program Files (x86)\e-Gulden\egulden-qt.exe" -zapwallettxes=1
) else if exist egulden-qt.exe (
    echo e-Gulden Core gevonden!
    echo We starten e-Gulden nu in een speciale modus. U hoeft dit programma niet nog eens uit te voeren.
    echo.
    echo Uw transactiegeschiedenis is nog niet zichtbaar. Geef de wallet de tijd om bij te werken.
    echo.
    egulden-qt.exe -zapwallettxes=1
) else (
    echo.
    echo Sorry, wij kunnen egulden-qt.exe niet vinden.
    echo.
    echo Zodra u op een toets drukt openen we een browser waarmee u de laatste versie kunt downloaden.
    echo.
    echo U kunt, zodra u de nieuwste versie geinstalleerd heeft, dit bestand opnieuw uitvoeren.
    echo.
    echo Indien u de nieuwste versie reeds geinstalleerd heeft en u deze melding blijft zien verzoeken wij u een mail te sturen naar foundation@e-gulden.org.
    echo.

    @pause

    @start "" https://github.com/Electronic-Gulden-Foundation/egulden/releases/latest
)

@pause
