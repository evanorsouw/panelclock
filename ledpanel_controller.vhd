library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity ledpanel_controller is
	port ( 	
		clk12M    : in std_logic;
		reset		 : in std_logic;
		uart_rx   : in std_logic;
		--
		dsp_clk   : out std_logic;
		dsp_latch : out std_logic;
		dsp_oe    : out std_logic;
		dsp_addr  : out std_logic_vector (4 downto 0);
		dsp_r1    : out std_logic;
		dsp_g1    : out std_logic;
		dsp_b1    : out std_logic;
		dsp_r2    : out std_logic;
		dsp_g2    : out std_logic;
		dsp_b2    : out std_logic;
		led1      : out std_logic;
		led2      : out std_logic
	);
end entity ledpanel_controller;

architecture ledpanel_controller_arch of ledpanel_controller is

	component pll
	port (
		inclk0	: in std_logic := '0';
		--
		c0			: out std_logic
	);
	end component;
	
	component FM6124
	port ( 	
		clk       : in std_logic;
		reset     : in std_logic;
		--
		addr      : out std_logic_vector (13 downto 0);
		dsp_clk   : out std_logic;
		dsp_latch : out std_logic;
		dsp_oe    : out std_logic;
		dsp_addr  : out std_logic_vector (4 downto 0);
		vbl       : out std_logic
	);
   end component;
	
	component dp_ram_32k
	PORT
	(
		data			: IN STD_LOGIC_VECTOR (5 DOWNTO 0);
		rdaddress	: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		rdclock		: IN STD_LOGIC;
		wraddress	: IN STD_LOGIC_VECTOR (15 DOWNTO 0);
		wrclock		: IN STD_LOGIC := '1';
		wren			: IN STD_LOGIC := '0';
		q				: OUT STD_LOGIC_VECTOR (5 DOWNTO 0)
	);
	end component;
	
	component UART
	PORT
	(
		clkx8			: in std_logic;
		reset			: in std_logic;
		rx				: in std_logic;
		datain  		: out std_logic_vector (7 downto 0);
		datain_clk	: out std_logic
	);
	end component;
	
	signal panel_reset    : std_logic := '0';
	signal clk            : std_logic;
	signal ram_rd_addr    : std_logic_vector (13 downto 0);
	signal ram_rd_data    : std_logic_vector (5 downto 0);
	signal uart_clk       : std_logic;
	signal uart_delay     : unsigned (15 downto 0);
	signal uart_datain    : std_logic_vector (7 downto 0);
	signal uart_dataclk   : std_logic;
	signal ram_wr_addr    : std_logic_vector(15 downto 0);
	signal ram_wr_data    : std_logic_vector (5 downto 0);
	signal ram_wr_clk     : std_logic;
	signal visible_page   : std_logic;
	signal vbl            : std_logic;
	signal dsp_base_addr  : std_logic_vector (15 downto 0);

begin
	LEDChip : FM6124
	port map (
		clk       => clk,
		reset     => panel_reset,
		--
		addr      => ram_rd_addr(13 downto 0),
		dsp_clk   => dsp_clk,
		dsp_latch => dsp_latch,
		dsp_addr  => dsp_addr,
		dsp_oe    => dsp_oe,
		vbl       => vbl
	);
	
	ClockGenerator : pll
	port map (
		inclk0  => clk12M,
		--
		c0 => clk
	);
	
	RAM : dp_ram_32k
	port map (
		rdaddress => "00" & ram_rd_addr,
		rdclock   => clk,
		data      => ram_wr_data,
		wraddress => ram_wr_addr,
		wrclock   => ram_wr_clk,
		wren      => '1',
		--
		q         => ram_rd_data
	);
	
	PC : UART
	port map (
		clkx8      => uart_clk,
		reset      => panel_reset,
		rx         => uart_rx,
		--
		datain     => uart_datain,
		datain_clk => uart_dataclk
	);
	
	MemRead_Proc: process (reset, clk)	
	begin
		panel_reset <= not reset;
		
		if rising_edge(clk) then
			dsp_r1 <= std_logic_vector(ram_rd_data)(0);
			dsp_g1 <= std_logic_vector(ram_rd_data)(1);
			dsp_b1 <= std_logic_vector(ram_rd_data)(2);
			dsp_r2 <= std_logic_vector(ram_rd_data)(3);
			dsp_g2 <= std_logic_vector(ram_rd_data)(4);
			dsp_b2 <= std_logic_vector(ram_rd_data)(5);
		end if;
		
		led1 <= uart_rx;
		led2 <= ram_wr_clk;
		
	end process;
	
	
	UARTClk_Proc: process (clk)
	variable counter : unsigned (8 downto 0);
	begin
		if rising_edge(clk) then
			counter := counter + 1;
			--if (counter = 391) then   -- 9600 Bd @8x oversampling @ 60MHz
			--if (counter = 195) then   -- 19200 Bd @8x oversampling @ 60MHz
			--if (counter = 98) then   -- 38400 Bd @8x oversampling @ 60MHz
			--if (counter = 49) then   -- 76800 Bd @8x oversampling @ 60MHz
			--if (counter = 33) then   -- 115200 Bd @8x oversampling @ 60MHz
			--if (counter = 16) then   -- 230400 Bd @8x oversampling @ 60MHz
			--if (counter = 8) then   -- 460800 Bd @8x oversampling @ 60MHz
			if (counter = 4) then   -- 921600 Bd @8x oversampling @ 60MHz
				counter := to_unsigned(0, counter'length);
				uart_clk <= not uart_clk;
			end if;
		end if;
	end process;
			
	UARTAPI_Proc: process (uart_dataclk)
	type T_APISTATE is ( START, DSPADDRHI, DSPADDRLO, ADDRHI, ADDRLO, PIXCOUNT, WRITING ); 
	variable state         : T_APISTATE;
	variable write_count   : unsigned (7 downto 0);
	variable write_address : unsigned (15 downto 0);
	begin
		if rising_edge(uart_dataclk) then
			
			case state is
			when START =>				
				if (unsigned(uart_datain) = X"01") then
					state := ADDRHI;
				elsif (unsigned(uart_datain) = X"02") then
					state := PIXCOUNT;
				elsif (unsigned(uart_datain) = X"03") then
					state := DSPADDRHI;
				end if;
			when DSPADDRHI =>
				dsp_base_addr(15 downto 8) <= std_logic_vector(uart_datain);
				state := DSPADDRLO;
			when DSPADDRLO =>
				dsp_base_addr(7 downto 0) <= std_logic_vector(uart_datain);
				state := START;
			when ADDRHI =>
				write_address(15 downto 8) := unsigned(uart_datain);
				state := ADDRLO;
			when ADDRLO =>
				write_address(7 downto 0) := unsigned(uart_datain);
				state := START;				
			when PIXCOUNT =>
				write_count := unsigned(uart_datain);
				state := WRITING;				
			when WRITING =>
				ram_wr_addr <= std_logic_vector(write_address);
				write_address := write_address + 1;
				ram_wr_data <= std_logic_vector(uart_datain)(5 downto 0);
				write_count := write_count - 1;
				if (write_count = 0) then
					state := START;
				end if;
			when others =>
				state := START;
			end case;
		end if;		
		ram_wr_clk <= uart_dataclk;
	end process;
	
end architecture ledpanel_controller_arch;
				
				
				
				