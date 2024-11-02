# Fechadura Inteligente com Arduino e ESP32

Ao acessar o site hospedado no GitHub Pages e fazer login, o usuário poderá acessar o painel e, assim, abrir ou fechar a fechadura inteligente de qualquer lugar do mundo.

Após o login do usuário em seu perfil, o site verificará no Firebase o estado da fechadura, exibindo a situação atual.

Quando ele clicar no botão, o JSON do Realtime Database do Firebase será modificado. Assim que essa alteração ocorrer, o ESP32 identificará a mudança e atualizará o estado da fechadura.

## Esquema de conexões do ESP32

#### Colocar a imagem

## Esquema de conexões do Arduino

#### Colocar a imagem