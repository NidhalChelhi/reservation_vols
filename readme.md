# Système de Réservation de Vols

Ceci est une application client-serveur implémentant un système de réservation de vols pour le mini-projet PSR (Programmation Système et Réseaux).

## Structure du Projet

Le projet comprend :

- `server.c` : Le composant serveur qui gère les requêtes de plusieurs agences clientes
- `client.c` : Le composant client représentant une agence de voyage
- `vols.txt` : Contient les informations des vols (référence, destination, sièges, prix)
- `histo.txt` : Enregistre l'historique des transactions
- `facture.txt` : Conserve les informations de facturation pour les agences

## Configuration Réseau

Dans cette configuration :

- Kali Linux (192.168.229.142) agit comme serveur
- Les autres machines virtuelles (RHEL, Rocky, Ubuntu) agissent comme agences clientes

## Compilation

Pour compiler le code, exécutez :

```
gcc server.c -o server -lpthread
```

```
gcc client.c -o client
```

Cela créera deux exécutables : `server` et `client`.

## Exécution de l'Application

1. D'abord, démarrez le serveur sur Kali Linux :

```
./server
```

2. Ensuite, démarrez un ou plusieurs clients sur les autres machines virtuelles :

```
./client
```

Lorsque vous y êtes invité, entrez un numéro d'agence (par exemple, 1, 2, 3)

Ou fournissez le numéro d'agence comme argument en ligne de commande :

```
./client 1
```

## Utilisation du Client

Le client fournit une interface à menu permettant aux agences de :

1. Voir tous les vols disponibles
2. Vérifier les informations d'un vol spécifique
3. Effectuer des réservations
4. Annuler des réservations
5. Voir la facture
6. Consulter l'historique des transactions

## Fonctionnalités du Serveur

Le serveur :

- Gère plusieurs connexions client simultanément en utilisant des threads
- Synchronise l'accès aux fichiers pour éviter la corruption des données
- Traite différents types de requêtes des agences
- Maintient les données persistantes entre les sessions

## Fichiers de Données

- `vols.txt` : Base de données des vols (mise à jour lors des réservations)
- `histo.txt` : Historique des transactions (mis à jour à chaque transaction)
- `facture.txt` : Informations de facturation des agences (mis à jour à chaque transaction)

## Notes d'Implémentation

- Utilise TCP pour une communication client-serveur fiable
- Utilise le threading pour gérer plusieurs connexions
- Met en œuvre un verrouillage de fichiers pour assurer la sécurité des threads
- Implémente un protocole simple basé sur du texte pour la communication
