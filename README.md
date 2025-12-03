# Unreal Engine – Plugin Collection (Multiplayer & Systems)

Conjunto de plugins desenvolvidos em **Unreal Engine 5**, focados em **multiplayer, replicação, gameplay systems** e criação de ferramentas reutilizáveis para agilizar e padronizar o desenvolvimento de jogos.

O objetivo deste projeto é fornecer soluções **prontas, escaláveis e fáceis de integrar**, tanto para projetos próprios quanto para comercialização futura.

Todos os sistemas foram pensados para serem:

- Modulares  
- Reutilizáveis  
- Totalmente replicados para multiplayer  
- Fáceis de adaptar  

## Tecnologias base

- Unreal Engine 5  
- C++  
- Blueprints  
- Networking / Replication  
- Estrutura voltada para performance e organização  

---

## 1. Replicated Inventory System

Sistema de inventário **totalmente replicado**, suportando múltiplos tipos de estrutura.

### Principais recursos

- Slots customizáveis  
- Replicação completa para multiplayer  
- Suporte para Quick Slot / Hotbar  
- Preparado para integração com loot, crafting e equipamentos  
- Estrutura flexível para diferentes tipos de jogos (RPG, Survival, Coop, etc)

Desenhado para funcionar tanto em projetos pequenos quanto em jogos mais complexos.

---

## 2. Generic MMO Monster System

Sistema completo para criação de monstros estilo MMO, totalmente **modular e expansível**, utilizando **Ability System Component** e componentes customizados.

### AggroComponent
Responsável por:

- Detectar o player automaticamente  
- Definir área de aggro (Aggro Range)  
- Executar habilidades de ataque  
- Fazer o monstro retornar ao ponto de spawn quando o alvo sai do alcance  

### HealthComponent
Responsável por:

- Gerenciar vida do monstro  
- Executar habilidades de morte  
- Permitir habilidades de cura  

### Ability System

- Integrado com o Ability System Component  
- Suporte para habilidades ofensivas e defensivas  
- Fácil de expandir com novas abilities  

### Spawner Manager (Actor customizado)

Sistema completo de spawn com alto nível de personalização:

- Definição do tamanho da área de spawn  
- Escolha dos tipos de monstros  
- Controle de chance de spawn por monstro  
- Padrões de spawn configuráveis  
- Controle de quantidade de mobs simultâneos  

Ideal para jogos estilo **RPG, MMO, Survival e Coop**.

---

## 3. Replicated Locomotion System

Sistema de locomoção **totalmente replicado** e **beginner-friendly**, desenvolvido para facilitar a criação de personagens multiplayer.

### Recursos principais

- Overlay States  
- Inputs customizáveis  
- Sistema de Attach Object (Blueprint ou Static Mesh)  
- Controle de Start / Stop Animations  
- Pivot Animations  
- Sistema configurável para First Person  
- Estrutura limpa e adaptável para diferentes tipos de jogo  

Projetado para ser fácil de integrar, modificar e expandir.

---

## Objetivo do projeto

Criar uma base sólida de sistemas reutilizáveis em Unreal Engine com foco em:

- Multiplayer e replicação  
- Redução de tempo de desenvolvimento  
- Padronização de estrutura de projeto  
- Facilidade de uso para outros desenvolvedores  
- Base para criação de templates e plugins comerciais  

**Status: Em desenvolvimento ativo**
