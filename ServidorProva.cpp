// Teste.cpp : Define o ponto de entrada para a aplicação de console.
// Server

/***Protocolo de comunicação
*Envio de mensagem: origem|destino|texto_
*Conexão: origem||_
*Desconexão: origem|_|_
*/

#include "stdafx.h"
#include <ctime>
#include <iostream>
#include <string>
#include <queue>
#include <array>
#include <map>
#include <list>
#include <boost\bind.hpp>
#include <boost\shared_ptr.hpp>
#include <boost\enable_shared_from_this.hpp>
#include <boost\filesystem\fstream.hpp>
#include <boost\algorithm\string.hpp>
#include <boost\thread.hpp>
#include <boost\asio.hpp>
#include <boost\asio\ip\tcp.hpp>


using boost::asio::ip::tcp;

std::string caracterSeparador = "|";
std::string caracterFimMsg = "_";
std::map<std::string, boost::asio::ip::tcp::socket&> usuario;

std::list <std::string> fila_envio;
void destrinche_msg(std::string, std::string&, std::string&, std::string&);
void envia_resposta(boost::asio::ip::tcp::socket&);
void empacotar_msg_enviar(std::string, std::string, std::string, boost::asio::ip::tcp::socket&);
void accept_handler(const boost::system::error_code&, std::string);
void on_connection(const boost::system::error_code, boost::asio::ip::tcp::socket&);
void log_arquivo(std::string);

int main()
{
	try
	{
		boost::asio::io_service io_service;
		boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), 4567);
		boost::asio::ip::tcp::acceptor acceptor(io_service, endpoint);
		while (1) {
			boost::asio::ip::tcp::socket socket(io_service);
			const boost::system::error_code error;
////			acceptor.async_accept(socket, endpoint, boost::bind(&on_connection, error, &socket));
			acceptor.async_accept(socket, endpoint, boost::bind(&on_connection, error, std::ref(socket)));
			//boost::asio::async_write(socket, boost::asio::buffer(vetor.front().data(),vetor.front().length()), boost::bind(&write_read_handler,error, tam));
			//socket.async_read_some(boost::asio::buffer(vetor, 32), boost::bind(&write_read_handler, error, tam));
			io_service.run();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
	system("PAUSE");
	return 0;
}

void destrinche_msg(std::string msg, std::string& orig, std::string& dest, std::string& corpo) {
	std::vector <std::string> tudo_separado;

	boost::split(tudo_separado, msg, boost::is_any_of(caracterSeparador));
	orig = tudo_separado.back(); tudo_separado.pop_back();
	std::cout << tudo_separado.back();
	dest = tudo_separado.back(); tudo_separado.pop_back();
	std::cout << tudo_separado.back();
	corpo = tudo_separado.back(); tudo_separado.pop_back();
	std::cout << tudo_separado.back();
}

void envia_resposta(boost::asio::ip::tcp::socket& socket) {
	/*?bloquear a fila*/
	try {
		const boost::system::error_code error;
		std::string resposta_string = fila_envio.back();
		//TODO transformar string -> array
		std::vector<char> resposta_vector(resposta_string.begin(), resposta_string.end());
//		socket.async_write_some(boost::asio::buffer(resposta_vector), boost::bind(&accept_handler, error, fila_envio.back()));
		socket.async_send(boost::asio::buffer(resposta_vector), boost::bind(&accept_handler, error, fila_envio.back()));
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void empacotar_msg_enviar(std::string orig, std::string dest, std::string corpo, boost::asio::ip::tcp::socket& socket) {
	std::string resposta = orig + caracterSeparador + dest + caracterSeparador + corpo + caracterFimMsg;//empacotar
	fila_envio.push_back(resposta);//enviar
////	boost::thread t(boost::bind(&envia_resposta, &socket));//enviar
	boost::thread t(boost::bind(&envia_resposta, std::ref(socket)));//enviar
	t.join();
}

void accept_handler(const boost::system::error_code& error, std::string texto) {
	try {
		if (!error)
		{
			log_arquivo(texto);
			std::cout << "sem erro leitura/escrita\n";
			fila_envio.pop_front();
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void on_connection(const boost::system::error_code error, boost::asio::ip::tcp::socket& socket)
{
	try {
		if (!error)
		{
			//while (1) {
				//std::cout << "sem erro leitura/escrita\n";
				std::array<char, 128> msg; msg.fill(' ');
////				socket.async_read_some(boost::asio::buffer(msg, 128), boost::bind(&accept_handler, &error, msg.data()));
				socket.async_read_some(boost::asio::buffer(msg, 128), boost::bind(&accept_handler, std::ref(error), msg.data()));
				std::string msgrecebida(msg.begin(), msg.end());
				std::string origem, destino, texto;
				boost::bind(&destrinche_msg, &msgrecebida, &origem, &destino, &texto);
				//if (origem == "") break;
				if (destino == "_") /*desconexão*/ {
					empacotar_msg_enviar(destino, origem, texto, socket);
					//break;
				}
				else if (destino == "") /*conexao*/
										/*disparar o semaforo pra acordar a thread enviar a msg*/
					empacotar_msg_enviar(destino, origem, texto, socket);
				else /*mandar msg*/
					empacotar_msg_enviar(destino, origem, texto, socket);

				std::cout << msg.data();
			//}
		}
	}
	catch (std::exception& e)
	{
		std::cerr << e.what() << std::endl;
	}
}

void log_arquivo(std::string texto) {
	boost::filesystem::path p{ "log.txt" };
	boost::filesystem::ofstream ofs{ p };
	ofs << texto << std::endl;
}
