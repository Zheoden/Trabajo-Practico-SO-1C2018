#Seteo todo a C
export LC_ALL=C
echo"Seteo a C realizado"

#Clono las commons 
echo "Git clone commons library"
git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library/
sudo make install
echo "Commons library instaladas correctamente"
cd

#Clono el parser
echo "Git clone commons library"
git clone https://github.com/sisoputnfrba/parsi.git
cd parsi/
sudo make install
echo "Parser instalado correctamente"
cd

#Clono el repositorio
mkdir Proyectos
cd Proyectos/
echo "Git clone del repositorio"
git clone https://github.com/iago64/tp-2018-1c-PC-citos.git 
echo "Repositorio instalado correctamente"
cd 


#Compilo librería
cd Proyectos/tp-2018-1c-PC-citos/liberia/Debug/
sudo make install
echo "Compilación librería ejecutada correctamente"
cd


echo "Compilando ESI..."
cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Debug/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/Proyectos/tp-2018-1c-PC-citos/libreria/Debug
make all
echo "Fin de la compilación ESI!"

echo "Compilando Instancia..."

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Debug/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/Proyectos/tp-2018-1c-PC-citos/libreria/Debug
make all
echo "Fin de la compilación INSTANCIA!" 

echo "Compilando Planificador..."
cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Debug/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/Proyectos/tp-2018-1c-PC-citos/libreria/Debug
make all
echo "Fin de la compilación PLANIFICADOR!"

echo "Compilando Coordinador..."
cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/Debug/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/home/utnso/Proyectos/tp-2018-1c-PC-citos/libreria/Debug
make all
echo "Fin de la compilación del COORDINADOR!"
