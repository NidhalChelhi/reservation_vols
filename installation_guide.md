# Guide d'Installation du Système de Réservation de Vols

Ce guide vous aidera à configurer le Système de Réservation de Vols sur vos machines virtuelles.

## Prérequis

Vous avez plusieurs machines virtuelles configurées dans VMware avec les adresses IP suivantes :

- Kali Linux: 192.168.229.142 (Serveur)
- RHEL: 192.168.229.136 (Client)
- Rocky: 192.168.229.140 (Client)
- Ubuntu: 192.168.229.141 (Client)

## Étapes d'Installation

### Sur le Serveur (Kali Linux)

1. Créez un répertoire pour le projet :

```
mkdir -p ~/reservation_vols
cd ~/reservation_vols
```

2. Copiez tous les fichiers depuis votre machine hôte vers ce répertoire :

   - server.c
   - client.c
   - vols.txt
   - histo.txt
   - facture.txt

3. Compilez le serveur :

```
gcc server.c -o server -lpthread
```

4. Si vous souhaitez aussi tester le client sur Kali :

```
gcc client.c -o client
```

### Sur les Machines Client (RHEL, Rocky, Ubuntu)

1. Créez un répertoire pour le projet :

```
mkdir -p ~/reservation_vols
cd ~/reservation_vols
```

2. Copiez les fichiers client vers ce répertoire :

   - client.c

3. Compilez le client :

```
gcc client.c -o client
```

## Exécution du Système

1. Démarrer d'abord le serveur sur Kali Linux :

```
cd ~/reservation_vols
./server
```

2. Puis démarrez les clients sur chaque machine virtuelle :

```
cd ~/reservation_vols
./client
```

Lorsque vous y êtes invité, entrez une référence d'agence (par exemple, 1 pour RHEL, 2 pour Rocky, 3 pour Ubuntu).

## Dépannage

Si vous rencontrez des problèmes de connexion :

1. Vérifiez si le serveur est en cours d'exécution :

```
ps aux | grep server
```

2. Vérifiez que l'IP du serveur est correctement définie dans le fichier client.c :

```
#define SERVER_IP "192.168.229.142"
```

3. Vérifiez si le port est ouvert sur le serveur :

```
netstat -tuln | grep 8080
```

4. Vérifiez la connectivité réseau du client vers le serveur :

```
ping 192.168.229.142
```

5. Vérifiez les problèmes de pare-feu :

   Sur Kali :

```
sudo iptables -L
```

Autorisez le trafic si nécessaire :

```
sudo iptables -A INPUT -p tcp --dport 8080 -j ACCEPT
```

## Structure des Fichiers

Assurez-vous que votre structure de répertoire ressemble à :

Sur le Serveur (Kali) :

```
~/reservation_vols/
  ├── server.c
  ├── client.c
  ├── vols.txt
  ├── histo.txt
  ├── facture.txt
  ├── server (exécutable)
  └── client (exécutable)
```

Sur les Clients (RHEL, Rocky, Ubuntu) :

```
~/reservation_vols/
  ├── client.c
  └── client (exécutable)
```
