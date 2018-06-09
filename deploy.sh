cd
mkdir Proyectos
cd
mv tp-2018-1c-PC-citos/ Proyectos/
cd

git clone https://github.com/sisoputnfrba/so-commons-library.git
cd so-commons-library/
sudo make install
cd

git clone https://github.com/sisoputnfrba/parsi.git
cd parsi/
sudo make install
cd

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/libreria/Debug
sudo make

sudo cp liblibreria.so /usr/lib/

cd 

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Coordinador/Debug/
make all
cd

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Instancia/Debug/
make all
cd

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/ESI/Debug/
make all
cd

cd /home/utnso/Proyectos/tp-2018-1c-PC-citos/Planificador/Debug/
make all
cd



