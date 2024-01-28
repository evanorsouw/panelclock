library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.numeric_std.all;

-- Not that this port signature is needed to enforce LSE to map the instance on a RAM block
-- See ICEcube2 User Guide on 'inferring RAM'
entity port2_ram is
   generic (
      DEPTH : natural := 8; -- 2^8 addresses
      WIDTH : natural := 8  -- 8 bits per address
   );
   port (
     -- read port
     i_rclk         : in std_logic;
     o_rdata        : out std_logic_vector(WIDTH-1 downto 0);
     i_raddr        : in std_logic_vector(DEPTH-1 downto 0);
     -- write port
     i_wclk         : in std_logic;
     i_wdata        : in std_logic_vector(WIDTH-1 downto 0);
     i_waddr        : in std_logic_vector(DEPTH-1 downto 0);
     i_wren         : in std_logic
   );
end entity;

architecture rtl of port2_ram is
   type tMem is array (2**DEPTH-1 downto 0) of std_logic_vector(WIDTH-1 downto 0);
   signal s_mem : tMem := (others => (others => '0'));
begin
   process (i_wclk)
   begin
      if rising_edge(i_wclk) and i_wren = '1' then
         s_mem(to_integer(unsigned(i_waddr))) <= i_wdata;
      end if;
   end process;
   
   process (i_rclk)
   begin
      if rising_edge(i_rclk) then
         o_rdata <= s_mem(to_integer(unsigned(i_raddr)));
      end if;
   end process;
end rtl;