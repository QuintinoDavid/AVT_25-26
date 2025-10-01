# AVT_25-26

__NOTA:__ No espaço de tempo entre a aula com o professor e a entrega, nós mudamos as colision boxes para serem mais dinamicas (ex. drone), e a câmera agora também se mexe quando o drone gira

**Perguntas das aulas práticas:**

__P:__ O ambiente precisa de ser gerado?  
__R:__ Não, é só meter uns cubos e cones.

__P:__ O que é o 'campo visual' para os objetos voadores?  
__R:__ Um raio arbitrário á volta do drone. Por exemplo, a 1000 unidades de distancia os objetos são eliminados.

__P:__ Os objetos voadores movem-se e rodam em que sentido?  
__R:__ O movimento pode ser uma simples linha reta, mas o modelo tem de sofrer alguma animação, por exemplo, rodar num dos eixos.

__P:__ A camera do drone segue a rotação do drone or fica fixa?  
__R:__ A camera segue a rotação do drone, sendo a sua posição em relação ao drone mudando com o movimento do drone, mas pode ser alterada com o movimento do rato.

__P:__ As texturas simultaneas são implementadas de que modo?  
__R:__ É simplesmente necessário aplicá-las ao chão e multiplicar diretamente no shader.


