---
title: "PList"
description: "Outil répliquant le comportement de pslist de sysinternals (windows)"
date: 2024-02-11T16:38:40.725Z
tags:
  - Programming
  - C
  - CMAKE
  - WINAPI
  - FORENSICS
---

# PList
Projet en C utilisant WINAPI afin de lister les processus en cours d'exécution sur un poste. 
Réalisé dans le cadre d'une main quest en binôme à l'Ecole 2600.

#### Auteurs
- Victor RENAULT
- Gwendal PATY

## Comment setup le projet
### Pré-requis
Il vous sera nécessaire de vérifier que chacun de ces outils sont installés sur votre appareil :
- [git](https://git-scm.com/download/win)
- [cmake](https://cmake.org/download/)
- [ninja](https://github.com/ninja-build/ninja/releases)

#### Releases
Vous pouvez également vous rendre sur notre repository où vous pourrez directement télécharger l'exécutable [ici](https://github.com/zxtNX/PList/releases/tag/v0.1.0).
### Etapes pour construire le projet et accéder à l'exécutable
#### Construction du projet
```bash
git clone git@github.com:zxtNX/PList.git
cd PList
build_release.bat
```
#### Accéder à l'exécutable et le lancer
Dans ``PList/build`` on lance ``PList.exe``.
```bash
cd build
PList.exe [options]
```
Le programme actuel ne comporte que l'argument ``-h``, ``-d`` et ``-m``.

**Si vous souhaitez accéder au maximum d'informations, il vous faudra exécuter l'utilitaire via Powershell en administrateur.**

## Ressources Windows Development utilisées
### Fondamentaux des Types de Données et APIs Windows
- [Types de données Windows](https://learn.microsoft.com/fr-fr/windows/win32/winprog/windows-data-types)
- [Structures de Données et APIs](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-luid)
- [Gestion des processus](https://learn.microsoft.com/en-us/windows/win32/procthread/process-security-and-access-rights)
- [Privileges et Sécurité](https://learn.microsoft.com/en-us/windows/win32/secauthz/privilege-constants)

### Interfaces de Programmation pour le Contrôle de Processus
- [Création d'un instantané des processus](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-createtoolhelp32snapshot)
- [Parcours des processus](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32first)
- [Suite du parcours des processus](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-process32next)
- [Entrée de processus](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-processentry32)

### Gestion et Accès aux Tokens de Sécurité
- [Ouverture de processus](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocess)
- [Manipulation des privilèges du token](https://learn.microsoft.com/en-us/windows/win32/api/securitybaseapi/nf-securitybaseapi-adjusttokenprivileges)
- [Structure des privilèges du token](https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-token_privileges)
- [Ouverture du token d'un processus](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openprocesstoken)

### Surveillance et Analyse des Processus
- [Information sur la mémoire d'un processus](https://learn.microsoft.com/en-us/windows/win32/api/psapi/nf-psapi-getprocessmemoryinfo)
- [Compteur de handle d'un processus](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesshandlecount)
- [Temps d'exécution d'un processus](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes)

### Gestion des Threads et Informations Système
- [Création et gestion des threads](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/nf-tlhelp32-thread32first)
- [Structure d'entrée des threads](https://learn.microsoft.com/en-us/windows/win32/api/tlhelp32/ns-tlhelp32-threadentry32)
- [Ouverture d'un thread](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-openthread)
- [Temps d'exécution d'un thread](https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getthreadtimes)
- [Information sur le système](https://learn.microsoft.com/en-us/windows/win32/api/winternl/nf-winternl-ntquerysysteminformation)
- [Étude détaillée de l'API système](https://www.geoffchappell.com/studies/windows/km/ntoskrnl/api/ex/sysinfo/process.htm)

### Protocoles et Erreurs Windows
- [Références des protocoles Windows](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-erref/596a1078-e883-4972-9bbc-49e60bebca55)
- [Autres spécifications de protocoles](https://learn.microsoft.com/en-us/openspecs/windows_protocols/ms-tsts/a11e7129-685b-4535-8d37-21d4596ac057)

### Ressources Complémentaires et Communauté
- [Discussion sur les APIs Windows](https://moustafasaleh.blogspot.com/2015/03/using-windows-native-apis-with-cl-and.html)
- [Questions et réponses sur StackOverflow](https://stackoverflow.com/questions/22949725/how-to-get-thread-state-e-g-suspended-memory-cpu-usage-start-time-priori)
