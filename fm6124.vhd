library IEEE;
use IEEE.STD_LOGIC_1164.all;
use IEEE.NUMERIC_STD.all;

entity FM6124 is
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
end entity FM6124;

architecture FM6124_arch of FM6124 is	
begin
	Clock_Proc: process (clk, reset)
	variable clk_count    : std_logic;
	variable column_count : unsigned (6 downto 0);
	variable row_count    : unsigned (4 downto 0);
	variable pixclk       : std_logic; 
	variable oe           : std_logic;
	variable latch        : std_logic;
	variable pixel_count  : unsigned (13 downto 0);
	variable waiting      : std_logic;
	variable depth_count  : unsigned (13 downto 0);
	variable depth_delay  : unsigned (6 downto 0);
	variable restart      : std_logic;          
	
	begin
		if (reset = '1') then
			clk_count    := '0';
			column_count := "0000000";
			row_count    := "00000";
			pixclk       := '0';
			latch        := '0';
			oe           := '1';
			waiting      := '0';
			depth_count  := "00000000000000";
			depth_delay  := "1000000";
			restart      := '1';
			
		elsif rising_edge(clk) then
			
			clk_count := not clk_count;
			restart   := '0';
			
			if (clk_count = '0') then
				pixclk  := '0';
				restart := '1';
								
			else
			   if (waiting = '1') then
					depth_count := depth_count - 1;
					if (depth_count = 0) then
						waiting := '0';
						if (row_count = 31) then
							depth_delay := depth_delay(0) & depth_delay(6 downto 1);
							if (depth_delay = "1000000") then 
								pixel_count := to_unsigned(0, pixel_count'length);
							end if;
						end if;
					end if;
				else
					-- repeat line while waiting instead of simply waiting idle
					if (column_count < 64) then 
						latch := '0';
						pixclk := '1';
						pixel_count := pixel_count + 1;
					elsif (column_count = 64) then
						latch  := '1';
					elsif (column_count = 65) then
						oe := '1';
						row_count := row_count + 1;
					elsif (column_count = 66) then
						oe := '0';
						waiting := '1';
						depth_count := depth_delay * 67;
						depth_count := depth_count - 66;
					end if;
					if (waiting = '1') then
						column_count := "0000000";
					else
						column_count := column_count + 1;
					end if;
				end if;
				
			end if;			
		end if;
		
		addr      <= std_logic_vector(pixel_count);
		dsp_clk   <= pixclk;
		dsp_oe    <= oe;
		dsp_latch <= latch;
		dsp_addr  <= std_logic_vector(row_count);
		vbl       <= restart;
		
	end process;
	
	
end architecture FM6124_arch;
				
				
				
				